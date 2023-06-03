#ifndef SERIAL_H_SEEN
#define SERIAL_H_SEEN 1

#include <unistd.h>             /* for usleep() and useconds_t, latter also might be via <sys/types.h> */
#include <sys/types.h>

ssize_t ser_send_char(TYPE_FD_SER fd, unsigned char ch);

/* send buflen bytes from buf with no delay */
ssize_t ser_send_buf(TYPE_FD_SER fd, const void *buf, size_t buflen);

ssize_t ser_get_char(TYPE_FD_SER fd, void *ch, time_t d_sec, useconds_t d_usec);

ssize_t ser_get_buf(TYPE_FD_SER fd, void *buf, size_t buflen, time_t d_sec, useconds_t d_usec);

/* keep reading until buflen bytes are received or a timeout occurs */
ssize_t ser_get_buf_len(TYPE_FD_SER fd, void *buf, size_t buflen, time_t d_sec, useconds_t d_usec);

ssize_t ser_flush_in(TYPE_FD_SER fd, const char *ignset, int verbose);

/* unified failure reporting: call these often */
void ser_comm_fail();

#endif	/* SERIAL_H_SEEN */