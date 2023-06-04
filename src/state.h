#include "upshandler.h"
#include <sys/types.h>

#define ST_MAX_VALUE_LEN 256

#define LARGEBUF	1024
#define SMALLBUF	512

/* state tree flags */
#define ST_FLAG_NONE      0x0000
#define ST_FLAG_RW        0x0001
#define ST_FLAG_STRING    0x0002 /* not STRING implies NUMBER */
#define ST_FLAG_NUMBER    0x0004
#define ST_FLAG_IMMUTABLE 0x0008

/* list of instant commands */
typedef struct cmdlist_s {
	char *name;
	struct cmdlist_s *next;
} cmdlist_t;


typedef struct st_tree_s {
	char	*var;
	char	*val;			/* points to raw or safe */

	char	*raw;			/* raw data from caller */
	size_t	rawsize;

	char	*safe;			/* safe data from pconf_encode */
	size_t	safesize;

	int	flags;
	long	aux;

	struct enum_s		*enum_list;
	struct range_s		*range_list;

	struct st_tree_s	*left;
	struct st_tree_s	*right;
} st_tree_t;

typedef struct enum_s {
	char	*val;
	struct enum_s	*next;
} enum_t;

typedef struct range_s {
	int min;
	int max;
	struct range_s	*next;
} range_t;

int state_setinfo(st_tree_t **nptr, const char *var, const char *val);
int state_setaux(st_tree_t *root, const char *var, const char *auxs);
const char *state_getinfo(st_tree_t *root, const char *var);
void state_setflags(st_tree_t *root, const char *var, size_t numflags, char **flags);
int state_addcmd(cmdlist_t **list, const char *cmd);

int dstate_setinfo(const char *var, const char *fmt, ...);
void dstate_setflags(const char *var, int flags);
void dstate_setaux(const char *var, long aux);
const char *dstate_getinfo(const char *var);
void dstate_addcmd(const char *cmdname);
void dstate_dataok(void);
void dstate_datastale(void);

int snprintfcat(char *dst, size_t size, const char *fmt, ...);


/* clean out the temp space for a new pass */
void status_init(void);

/* add a status element */
void status_set(const char *buf);

/* write the temporary status_buf into ups.status */
void status_commit(void);

/* similar functions for ups.alarm */
void alarm_init(void);
void alarm_set(const char *buf);
void alarm_commit(void);

