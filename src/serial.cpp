/* serial.c - common serial port functions for Network UPS Tools drivers

   Copyright (C) 2003  Russell Kroll <rkroll@exploits.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "HardwareSerial.h"

//#include <grp.h>
//#include <sys/ioctl.h>

//#include <ctype.h>
//#include <sys/file.h>
//#include <sys/types.h>
//#include <unistd.h>

#include "nut_stdint.h"


HardwareSerial fd(TYPE_FD_SER);

static unsigned int	comm_failures = 0;



/* Non fatal version of ser_open */
void ser_open(unsigned long speed)
{

	fd.begin(speed); 

	//return fd;
}


void ser_close()
{
  fd.end;
}

ssize_t ser_send_char(TYPE_FD_SER fd, unsigned char ch)
{
	return ser_send_buf_pace(fd, 0, &ch, 1);
}

static ssize_t send_formatted(TYPE_FD_SER fd, const char *fmt, va_list va, useconds_t d_usec)
{
	int	ret;
	char	buf[LARGEBUF];

#ifdef HAVE_PRAGMAS_FOR_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic push
#endif
#ifdef HAVE_PRAGMA_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
#ifdef HAVE_PRAGMA_GCC_DIAGNOSTIC_IGNORED_FORMAT_SECURITY
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
	ret = vsnprintf(buf, sizeof(buf), fmt, va);
#ifdef HAVE_PRAGMAS_FOR_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic pop
#endif

	if (ret >= (int)sizeof(buf)) {
		Serial.printf("vsnprintf needed more than %d bytes", (int)sizeof(buf));
	}

	return ser_send_buf_pace(fd, d_usec, buf, strlen(buf));
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
	const char	*data = (char *) buf;

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
	char	*data = (char *) buf;

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
			Serial.printf("ser_flush_in: read char %c",	ch);
		else
			Serial.printf("ser_flush_in: read char 0x%02x", ch);
	}

	return extra;
}

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

/* Write up to buflen bytes to fd and return the number of bytes
   written. If no data is available within d_sec + d_usec, return 0.
   On error, a value < 0 is returned (errno indicates error). */

ssize_t select_write(const int fd, const void *buf, const size_t buflen, const time_t d_sec, const suseconds_t d_usec)
{
	int		ret;
	fd_set		fds;
	struct timeval	tv;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	tv.tv_sec = d_sec;
	tv.tv_usec = d_usec;

	ret = select(fd + 1, NULL, &fds, NULL, &tv);

	if (ret < 1) {
		return ret;
	}

	return write(fd, buf, buflen);
}
