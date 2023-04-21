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

#include "common.h"
#include "serial.h"
#include "main.h"

#include <grp.h>
#include <pwd.h>
#include <sys/ioctl.h>

#include <ctype.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_UU_LOCK
#include <libutil.h>
#endif

#include "nut_stdint.h"

	static unsigned int	comm_failures = 0;



/* Non fatal version of ser_open */
TYPE_FD_SER ser_open_nf(const char *port)
{
	TYPE_FD_SER	fd;

	fd = open(port, O_RDWR | O_NOCTTY | O_EXCL | O_NONBLOCK);

	if (INVALID_FD_SER(fd)) {
		return ERROR_FD_SER;
	}


	return fd;
}

TYPE_FD_SER ser_open(const char *port)
{
	TYPE_FD_SER res;

	res = ser_open_nf(port);
	if (INVALID_FD_SER(res)) {
		//message about unable to open
	}

	return res;
}

int ser_set_speed_nf(TYPE_FD_SER fd, const char *port, speed_t speed)
{
	struct	termios	tio;
	NUT_UNUSED_VARIABLE(port);

	if (tcgetattr(fd, &tio) != 0) {
		return -1;
	}

	tio.c_cflag = CS8 | CLOCAL | CREAD;
	tio.c_iflag = IGNPAR;
	tio.c_oflag = 0;
	tio.c_lflag = 0;
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	cfsetispeed(&tio, speed);
	cfsetospeed(&tio, speed);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &tio);

	return 0;
}

int ser_set_speed(TYPE_FD_SER fd, const char *port, speed_t speed)
{
	int res;

	res = ser_set_speed_nf(fd,port,speed);
	if(res == -1) {
		Serial.printf("tcgetattr(%s)", port);
	}

	return 0;
}


int ser_close(TYPE_FD_SER fd, const char *port)
{
	if (INVALID_FD_SER(fd)) {
		Serial.printf("ser_close: programming error: fd=%d port=%s", fd, port);
	}

	if (close(fd) != 0)
		return -1;

#ifdef HAVE_UU_LOCK
	if (do_lock_port)
		uu_unlock(xbasename(port));
#endif

	return 0;
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

/* send the results of the format string with d_usec delay after each char */
ssize_t ser_send_pace(TYPE_FD_SER fd, useconds_t d_usec, const char *fmt, ...)
{
	ssize_t	ret;
	va_list	ap;

	va_start(ap, fmt);

#ifdef HAVE_PRAGMAS_FOR_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic push
#endif
#ifdef HAVE_PRAGMA_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
#ifdef HAVE_PRAGMA_GCC_DIAGNOSTIC_IGNORED_FORMAT_SECURITY
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
	ret = send_formatted(fd, fmt, ap, d_usec);
#ifdef HAVE_PRAGMAS_FOR_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic pop
#endif

	va_end(ap);

	return ret;
}

/* send the results of the format string with no delay */
ssize_t ser_send(TYPE_FD_SER fd, const char *fmt, ...)
{
	ssize_t	ret;
	va_list	ap;

	va_start(ap, fmt);

#ifdef HAVE_PRAGMAS_FOR_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic push
#endif
#ifdef HAVE_PRAGMA_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
#ifdef HAVE_PRAGMA_GCC_DIAGNOSTIC_IGNORED_FORMAT_SECURITY
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
	ret = send_formatted(fd, fmt, ap, 0);
#ifdef HAVE_PRAGMAS_FOR_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic pop
#endif

	va_end(ap);

	return ret;
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

/* reads a line up to <endchar>, discarding anything else that may follow,
   with callouts to the handler if anything matches the alertset */
ssize_t ser_get_line_alert(TYPE_FD_SER fd, void *buf, size_t buflen, char endchar,
	const char *ignset, const char *alertset, void handler(char ch),
	time_t d_sec, useconds_t d_usec)
{
	ssize_t	i, ret;
	char	tmp[64];
	char	*data = (char *) buf;
	ssize_t	count = 0, maxcount;

	assert(buflen < SSIZE_MAX && buflen > 0);
	memset(buf, '\0', buflen);

	maxcount = (ssize_t)buflen - 1;		/* for trailing \0 */

	while (count < maxcount) {
		ret = select_read(fd, tmp, sizeof(tmp), d_sec, (suseconds_t)d_usec);

		if (ret < 1) {
			return ret;
		}

		for (i = 0; i < ret; i++) {

			if ((count == maxcount) || (tmp[i] == endchar)) {
				return count;
			}

			if (strchr(ignset, tmp[i]))
				continue;

			if (strchr(alertset, tmp[i])) {
				if (handler)
					handler(tmp[i]);

				continue;
			}

			data[count++] = tmp[i];
		}
	}

	return count;
}

/* as above, only with no alertset handling (just a wrapper) */
ssize_t ser_get_line(TYPE_FD_SER fd, void *buf, size_t buflen, char endchar,
	const char *ignset, time_t d_sec, useconds_t d_usec)
{
	return ser_get_line_alert(fd, buf, buflen, endchar, ignset, "", NULL,
		d_sec, d_usec);
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

int ser_flush_io(TYPE_FD_SER fd)
{
	return tcflush(fd, TCIOFLUSH);
}

void ser_comm_fail(const char *fmt, ...)
{
	int	ret;
	char	why[SMALLBUF];
	va_list	ap;

	/* this means we're probably here because select was interrupted */
	
	comm_failures++;

	if ((comm_failures == SER_ERR_LIMIT) ||
		((comm_failures % SER_ERR_RATE) == 0)) {
		Serial.printf("Warning: excessive comm failures, "
			"limiting error reporting");
	}

	/* once it's past the limit, only log once every SER_ERR_LIMIT calls */
	if ((comm_failures > SER_ERR_LIMIT) &&
		((comm_failures % SER_ERR_LIMIT) != 0))
		return;

	va_start(ap, fmt);
#ifdef HAVE_PRAGMAS_FOR_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic push
#endif
#ifdef HAVE_PRAGMA_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
#ifdef HAVE_PRAGMA_GCC_DIAGNOSTIC_IGNORED_FORMAT_SECURITY
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
	ret = vsnprintf(why, sizeof(why), fmt, ap);
#ifdef HAVE_PRAGMAS_FOR_GCC_DIAGNOSTIC_IGNORED_FORMAT_NONLITERAL
#pragma GCC diagnostic pop
#endif
	va_end(ap);

	if ((ret < 1) || (ret >= (int) sizeof(why)))
		Serial.printf("ser_comm_fail: vsnprintf needed "
			"more than %d bytes", (int)sizeof(why));

	Serial.printf("Communications with UPS lost: %s", why);
}

void ser_comm_good(void)
{
	if (comm_failures == 0)
		return;

	Serial.printf("Communications with UPS re-established");
	comm_failures = 0;
}