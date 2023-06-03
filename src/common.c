#include "common.h"

ssize_t select_read(const int fd, void *buf, const size_t buflen, const time_t d_sec, const suseconds_t d_usec)
{
	int		ret;
	fd_set		fds;
	struct timeval	tv;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	tv.tv_sec = d_sec;
	tv.tv_usec = d_usec;

	ret = select(fd + 1, &fds, NULL, NULL, &tv);

	if (ret < 1) {
		return ret;
	}

	return read(fd, buf, buflen);
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

void *xmalloc(size_t size)
{
	void *p = malloc(size);

	if (p == NULL)
		fatal_with_errno(EXIT_FAILURE, "%s", oom_msg);
	return p;
}
