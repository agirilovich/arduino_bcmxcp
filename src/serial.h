#include <unistd.h>             /* for usleep() and useconds_t, latter also might be via <sys/types.h> */
#include <sys/types.h>

/* limit the amount of spew that goes in the syslog when we lose the UPS */
#define SER_ERR_LIMIT 10	/* start limiting after 10 in a row  */
#define SER_ERR_RATE 100	/* then only print every 100th error */

void ser_open(unsigned long speed);

void ser_close();

ssize_t ser_send_char(TYPE_FD_SER fd, unsigned char ch);

/* send the results of the format string with d_usec delay after each char */
/* send buflen bytes from buf with no delay */
ssize_t ser_send_buf(TYPE_FD_SER fd, const void *buf, size_t buflen);

/* send buflen bytes from buf with d_usec delay after each char */
ssize_t ser_send_buf_pace(TYPE_FD_SER fd, useconds_t d_usec, const void *buf,
	size_t buflen);

ssize_t ser_get_char(TYPE_FD_SER fd, void *ch, time_t d_sec, useconds_t d_usec);

ssize_t ser_get_buf(TYPE_FD_SER fd, void *buf, size_t buflen, time_t d_sec, useconds_t d_usec);

/* keep reading until buflen bytes are received or a timeout occurs */
ssize_t ser_get_buf_len(TYPE_FD_SER fd, void *buf, size_t buflen, time_t d_sec, useconds_t d_usec);

ssize_t ser_flush_in(TYPE_FD_SER fd, const char *ignset, int verbose);

/* Note: different method signatures instead of TYPE_FD_SER due to "const" */
ssize_t select_read(const int fd, void *buf, const size_t buflen, const time_t d_sec, const suseconds_t d_usec);
ssize_t select_write(const int fd, const void *buf, const size_t buflen, const time_t d_sec, const suseconds_t d_usec);
