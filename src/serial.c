#include "main.h"
#include "serial.h"

ssize_t ser_send_char(TYPE_FD_SER fd, unsigned char ch)
{
	return ser_send_buf_pace(fd, 0, &ch, 1);
}


/* send buflen bytes from buf with no delay */
ssize_t ser_send_buf(TYPE_FD_SER fd, const void *buf, size_t buflen)
{
	return ser_send_buf_pace(fd, 0, buf, buflen);
}

/* send buflen bytes from buf with d_usec delay after each char */
ssize_t ser_send_buf_pace(TYPE_FD_SER fd, useconds_t d_usec, const void *buf,
	size_t buflen)
{
	ssize_t	ret = 0;
	ssize_t	sent;
	const char	*data = buf;

	assert(buflen < SSIZE_MAX);
	for (sent = 0; sent < (ssize_t)buflen; sent += ret) {
		/* Conditions above ensure that (buflen - sent) > 0 below */
		ret = write(fd, &data[sent], (d_usec == 0) ? (size_t)((ssize_t)buflen - sent) : 1);

		if (ret < 1) {
			return ret;
		}

		usleep(d_usec);
	}

	return sent;
}

ssize_t ser_get_char(TYPE_FD_SER fd, void *ch, time_t d_sec, useconds_t d_usec)
{
	/* Per standard below, we can cast here, because required ranges are
	 * effectively the same (and signed -1 for suseconds_t), and at most long:
	 * https://pubs.opengroup.org/onlinepubs/009604599/basedefs/sys/types.h.html
	 */
	return select_read(fd, ch, 1, d_sec, (suseconds_t)d_usec);
}

ssize_t ser_get_buf(TYPE_FD_SER fd, void *buf, size_t buflen, time_t d_sec, useconds_t d_usec)
{
	memset(buf, '\0', buflen);

	return select_read(fd, buf, buflen, d_sec, (suseconds_t)d_usec);
}

/* keep reading until buflen bytes are received or a timeout occurs */
ssize_t ser_get_buf_len(TYPE_FD_SER fd, void *buf, size_t buflen, time_t d_sec, useconds_t d_usec)
{
	ssize_t	ret;
	ssize_t	recv;
	char	*data = buf;

	assert(buflen < SSIZE_MAX);
	memset(buf, '\0', buflen);

	for (recv = 0; recv < (ssize_t)buflen; recv += ret) {

		ret = select_read(fd, &data[recv],
			(size_t)((ssize_t)buflen - recv),
			d_sec, (suseconds_t)d_usec);

		if (ret < 1) {
			return ret;
		}
	}

	return recv;
}

ssize_t ser_flush_in(TYPE_FD_SER fd, const char *ignset, int verbose)
{
	ssize_t	ret, extra = 0;
	char	ch;

	while ((ret = ser_get_char(fd, &ch, 0, 0)) > 0) {

		if (strchr(ignset, ch))
			continue;

		extra++;

		if (verbose == 0)
			continue;

		if (isprint((unsigned char)ch & 0xFF))
			upslogx(LOG_INFO, "ser_flush_in: read char %c",	ch);
		else
			upslogx(LOG_INFO, "ser_flush_in: read char 0x%02x", ch);
	}

	return extra;
}


void ser_comm_fail()
{
	Serial.println("Communications with UPS lost");
}
