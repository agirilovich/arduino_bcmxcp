#include "state.h"
#include <strings.h>
#include <Arduino.h>

static cmdlist_t *cmdhead = NULL;
static st_tree_t	*dtree_root = NULL;
static int	stale = 1, alarm_active = 0, ignorelb = 0;
static char	status_buf[ST_MAX_VALUE_LEN], alarm_buf[ST_MAX_VALUE_LEN];
size_t buflen;

void dstate_addcmd(const char *cmdname)
{
	int	ret;

	ret = state_addcmd(&cmdhead, cmdname);

}

int state_addcmd(cmdlist_t **list, const char *cmd)
{
	cmdlist_t *item;

	while (*list) {

		if (strcasecmp((*list)->name, cmd) > 0) {
			/* insertion point reached */
			break;
		}

		if (strcasecmp((*list)->name, cmd) < 0) {
			list = &(*list)->next;
			continue;
		}

		return 0;	/* duplicate */
	}

	item = (cmdlist_t*)malloc(sizeof(*item));
	item->name = strdup(cmd);
	item->next = *list;

	/* now we're done creating it, insert it in the list */
	*list = item;

	return 1;	/* added */
}

int dstate_setinfo(const char *var, const char *fmt, ...)
{
	int	ret;
	char	value[ST_MAX_VALUE_LEN];
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(value, sizeof(value), fmt, ap);
	va_end(ap);

	ret = state_setinfo(&dtree_root, var, value);

	return ret;
}

int state_setinfo(st_tree_t **nptr, const char *var, const char *val)
{
	while (*nptr) {

		st_tree_t	*node = *nptr;

		if (strcasecmp(node->var, var) > 0) {
			nptr = &node->left;
			continue;
		}

		if (strcasecmp(node->var, var) < 0) {
			nptr = &node->right;
			continue;
		}

		/* updating an existing entry */
		if (!strcasecmp(node->raw, val)) {
			return 0;	/* no change */
		}

		/* changes should be ignored */
		if (node->flags & ST_FLAG_IMMUTABLE) {
			return 0;	/* no change */
		}

		/* expand the buffer if the value grows */
		if (node->rawsize < (strlen(val) + 1)) {
			node->rawsize = strlen(val) + 1;
			node->raw = (char*)realloc(node->raw, node->rawsize);
		}

		/* store the literal value for later comparisons */
		snprintf(node->raw, node->rawsize, "%s", val);

		return 1;	/* changed */
	}

	*nptr = (st_tree_t*)malloc(sizeof(**nptr));

	(*nptr)->var = strdup(var);
	(*nptr)->raw = strdup(val);
	(*nptr)->rawsize = strlen(val) + 1;

	return 1;	/* added */
}

st_tree_t *state_tree_find(st_tree_t *node, const char *var)
{
	while (node) {

		if (strcasecmp(node->var, var) > 0) {
			node = node->left;
			continue;
		}

		if (strcasecmp(node->var, var) < 0) {
			node = node->right;
			continue;
		}

		break;	/* found */
	}

	return node;
}

int state_setaux(st_tree_t *root, const char *var, const char *auxs)
{
	st_tree_t	*sttmp;
	long	aux;

	/* find the tree node for var */
	sttmp = state_tree_find(root, var);

	if (!sttmp) {
		return -1;	/* failed */
	}

	aux = strtol(auxs, (char **) NULL, 10);

	/* silently ignore matches */
	if (sttmp->aux == aux) {
		return 0;
	}

	sttmp->aux = aux;

	return 1;
}

const char *state_getinfo(st_tree_t *root, const char *var)
{
	st_tree_t	*sttmp;

	/* find the tree node for var */
	sttmp = state_tree_find(root, var);

	if (!sttmp) {
		return NULL;
	}

	return sttmp->val;
}

void state_setflags(st_tree_t *root, const char *var, size_t numflags, char **flag)
{
	size_t	i;
	st_tree_t	*sttmp;

	/* find the tree node for var */
	sttmp = state_tree_find(root, var);

	if (!sttmp) {
		return;
	}

	sttmp->flags = 0;

	for (i = 0; i < numflags; i++) {

		if (!strcasecmp(flag[i], "RW")) {
			sttmp->flags |= ST_FLAG_RW;
			continue;
		}

		if (!strcasecmp(flag[i], "STRING")) {
			sttmp->flags |= ST_FLAG_STRING;
			continue;
		}

		if (!strcasecmp(flag[i], "NUMBER")) {
			sttmp->flags |= ST_FLAG_NUMBER;
			continue;
		}
	}
}

void dstate_setflags(const char *var, int flags)
{
	st_tree_t	*sttmp;
	char	flist[SMALLBUF];

	/* find the dtree node for var */
	sttmp = state_tree_find(dtree_root, var);

	if (!sttmp) {
		return;
	}

	if (sttmp->flags & ST_FLAG_IMMUTABLE) {
		return;
	}

	if (sttmp->flags == flags) {
		return;		/* no change */
	}

	sttmp->flags = flags;

	/* build the list */
	snprintf(flist, sizeof(flist), "%s", var);

	if (flags & ST_FLAG_RW) {
		snprintfcat(flist, sizeof(flist), " RW");
	}

	if (flags & ST_FLAG_STRING) {
		snprintfcat(flist, sizeof(flist), " STRING");
	}

	if (flags & ST_FLAG_NUMBER) {
		snprintfcat(flist, sizeof(flist), " NUMBER");
	}

}

void dstate_setaux(const char *var, long aux)
{
	st_tree_t	*sttmp;

	/* find the dtree node for var */
	sttmp = state_tree_find(dtree_root, var);

	if (!sttmp) {
		return;
	}

	if (sttmp->aux == aux) {
		return;		/* no change */
	}

	sttmp->aux = aux;
}

const char *dstate_getinfo(const char *var)
{
	return state_getinfo(dtree_root, var);
}

void dstate_dataok(void)
{
	if (stale == 1) {
		stale = 0;
	}
}

void dstate_datastale(void)
{
	if (stale == 0) {
		stale = 1;
	}
}

void status_init(void)
{
	if (dstate_getinfo("driver.flag.ignorelb")) {
		ignorelb = 1;
	}

	memset(status_buf, 0, sizeof(status_buf));
}

void status_set(const char *buf)
{
	if (ignorelb && !strcasecmp(buf, "LB")) {
		return;
	}

	/* separate with a space if multiple elements are present */
	if (strlen(status_buf) > 0) {
		snprintfcat(status_buf, sizeof(status_buf), " %s", buf);
	} else {
		snprintfcat(status_buf, sizeof(status_buf), "%s", buf);
	}
}

void status_commit(void)
{
	while (ignorelb) {
		const char	*val, *low;

		val = dstate_getinfo("battery.charge");
		low = dstate_getinfo("battery.charge.low");

		if (val && low && (strtol(val, NULL, 10) < strtol(low, NULL, 10))) {
			snprintfcat(status_buf, sizeof(status_buf), " LB");
			break;
		}

		val = dstate_getinfo("battery.runtime");
		low = dstate_getinfo("battery.runtime.low");

		if (val && low && (strtol(val, NULL, 10) < strtol(low, NULL, 10))) {
			snprintfcat(status_buf, sizeof(status_buf), " LB");
			break;
		}

		/* LB condition not detected */
		break;
	}

	if (alarm_active) {
		dstate_setinfo("ups.status", "ALARM %s", status_buf);
	} else {
		dstate_setinfo("ups.status", "%s", status_buf);
	}
}

void alarm_init(void)
{
	/* reinit global counter */
	alarm_active = 0;

	memset(alarm_buf, 0, sizeof(alarm_buf));
}

void alarm_set(const char *buf)
{
	int ret;
	if (strlen(alarm_buf) > 0) {
		ret = snprintfcat(alarm_buf, sizeof(alarm_buf), " %s", buf);
	} else {
		ret = snprintfcat(alarm_buf, sizeof(alarm_buf), "%s", buf);
	}

	if (ret < 0) {
		/* Should we also try to print the potentially unusable buf?
		 * Generally - likely not. But if it is short enough...
		 * Note: LARGEBUF was the original limit mismatched vs alarm_buf
		 * size before PR #986.
		 */
		char alarm_tmp[LARGEBUF];
		memset(alarm_tmp, 0, sizeof(alarm_tmp));
		/* A bit of complexity to keep both (int)snprintf(...) and (size_t)sizeof(...) happy */
		int ibuflen = snprintf(alarm_tmp, sizeof(alarm_tmp), "%s", buf);
		size_t buflen;
		if (ibuflen < 0) {
			alarm_tmp[0] = 'N';
			alarm_tmp[1] = '/';
			alarm_tmp[2] = 'A';
			alarm_tmp[3] = '\0';
			buflen = strlen(alarm_tmp);
		} else {
			if ((unsigned long long int)ibuflen < SIZE_MAX) {
			  buflen = (size_t)ibuflen;
		    } else {
              buflen = SIZE_MAX;
		    }
		}
	} else if ((size_t)ret > sizeof(alarm_buf)) {
		char alarm_tmp[LARGEBUF];
		memset(alarm_tmp, 0, sizeof(alarm_tmp));
		int ibuflen = snprintf(alarm_tmp, sizeof(alarm_tmp), "%s", buf);
		size_t buflen;
		if (ibuflen < 0) {
			alarm_tmp[0] = 'N';
			alarm_tmp[1] = '/';
			alarm_tmp[2] = 'A';
			alarm_tmp[3] = '\0';
			buflen = strlen(alarm_tmp);
		} else {
			if ((unsigned long long int)ibuflen < SIZE_MAX) {
				buflen = (size_t)ibuflen;
			} else {
				buflen = SIZE_MAX;
			}
		}
	}
}

static void st_tree_node_add(st_tree_t **nptr, st_tree_t *sptr)
{
	if (!sptr) {
		return;
	}

	while (*nptr) {

		st_tree_t	*node = *nptr;

		if (strcasecmp(node->var, sptr->var) > 0) {
			nptr = &node->left;
			continue;
		}

		if (strcasecmp(node->var, sptr->var) < 0) {
			nptr = &node->right;
			continue;
		}
		return;
	}

	*nptr = sptr;
}


static void st_tree_enum_free(enum_t *list)
{
	if (!list) {
		return;
	}

	st_tree_enum_free(list->next);

	free(list->val);
	free(list);
}

static void st_tree_range_free(range_t *list)
{
	if (!list) {
		return;
	}

	st_tree_range_free(list->next);

	free(list);
}

static void st_tree_node_free(st_tree_t *node)
{
	free(node->var);
	free(node->raw);
	free(node->safe);

	/* never free node->val, since it's just a pointer to raw or safe */

	/* blow away the list of enums */
	st_tree_enum_free(node->enum_list);

	/* and the list of ranges */
	st_tree_range_free(node->range_list);

	/* now finally kill the node itself */
	free(node);
}

int state_delinfo(st_tree_t **nptr, const char *var)
{
	while (*nptr) {

		st_tree_t	*node = *nptr;

		if (strcasecmp(node->var, var) > 0) {
			nptr = &node->left;
			continue;
		}

		if (strcasecmp(node->var, var) < 0) {
			nptr = &node->right;
			continue;
		}

		if (node->flags & ST_FLAG_IMMUTABLE) {
			return 0;
		}

		/* whatever is on the left, hang it off current right */
		st_tree_node_add(&node->right, node->left);

		/* now point the parent at the old right child */
		*nptr = node->right;

		st_tree_node_free(node);

		return 1;
	}

	return 0;	/* not found */
}

int dstate_delinfo(const char *var)
{
	int	ret;

	ret = state_delinfo(&dtree_root, var);

	return ret;
}

void alarm_commit(void)
{
	if (strlen(alarm_buf) > 0) {
		dstate_setinfo("ups.alarm", "%s", alarm_buf);
		alarm_active = 1;
	} else {
		dstate_delinfo("ups.alarm");
		alarm_active = 0;
	}
}

int snprintfcat(char *dst, size_t size, const char *fmt, ...)
{
	va_list ap;
	size_t len = strlen(dst);
	int ret;

	size--;
	if (len > size) {
		/* Do not truncate existing string */
		return -1;
	}

	va_start(ap, fmt);
	ret = vsnprintf(dst + len, size - len, fmt, ap);
	va_end(ap);

	dst[size] = '\0';

	if (ret < 0) {
		return ret;
	}
	if ( ( (unsigned long long)len + (unsigned long long)ret ) >= (unsigned long long)INT_MAX ) {
		return -1;
	}
	return (int)len + ret;
}