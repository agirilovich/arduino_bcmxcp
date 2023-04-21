/* common.c - common useful functions

   Copyright (C) 2000  Russell Kroll <rkroll@exploits.org>
   Copyright (C) 2021-2022  Jim Klimov <jimklimov+nut@gmail.com>

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

#include <ctype.h>

/* Similarly for only reporting once if the notification subsystem is not built-in */
static int upsnotify_reported_disabled_notech = 0;

/* the reason we define UPS_VERSION as a static string, rather than a
	macro, is to make dependency tracking easier (only common.o depends
	on nut_version_macro.h), and also to prevent all sources from
	having to be recompiled each time the version changes (they only
	need to be re-linked). */

#include <stdio.h>

/* Know which bitness we were built for,
 * to adjust the search paths for get_libname() */
#include "nut_stdint.h"
#if defined(UINTPTR_MAX) && (UINTPTR_MAX + 0) == 0xffffffffffffffffULL
# define BUILD_64   1
#else
# ifdef BUILD_64
#  undef BUILD_64
# endif
#endif

/* https://stackoverflow.com/a/12844426/4715872 */
#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>


/* taken from www.asciitable.com */
static const char* ascii_symb[] = {
	"NUL",  /*  0x00    */
	"SOH",  /*  0x01    */
	"STX",  /*  0x02    */
	"ETX",  /*  0x03    */
	"EOT",  /*  0x04    */
	"ENQ",  /*  0x05    */
	"ACK",  /*  0x06    */
	"BEL",  /*  0x07    */
	"BS",   /*  0x08    */
	"TAB",  /*  0x09    */
	"LF",   /*  0x0A    */
	"VT",   /*  0x0B    */
	"FF",   /*  0x0C    */
	"CR",   /*  0x0D    */
	"SO",   /*  0x0E    */
	"SI",   /*  0x0F    */
	"DLE",  /*  0x10    */
	"DC1",  /*  0x11    */
	"DC2",  /*  0x12    */
	"DC3",  /*  0x13    */
	"DC4",  /*  0x14    */
	"NAK",  /*  0x15    */
	"SYN",  /*  0x16    */
	"ETB",  /*  0x17    */
	"CAN",  /*  0x18    */
	"EM",   /*  0x19    */
	"SUB",  /*  0x1A    */
	"ESC",  /*  0x1B    */
	"FS",   /*  0x1C    */
	"GS",   /*  0x1D    */
	"RS",   /*  0x1E    */
	"US"    /*  0x1F    */
};

/* Read up to buflen bytes from fd and return the number of bytes
   read. If no data is available within d_sec + d_usec, return 0.
   On error, a value < 0 is returned (errno indicates error). */
#ifndef WIN32
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
#else
ssize_t select_read(serial_handler_t *fd, void *buf, const size_t buflen, const time_t d_sec, const suseconds_t d_usec)
{
	/* This function is only called by serial drivers right now */
	/* TODO: Assert below that resulting values fit in ssize_t range */
	/* DWORD bytes_read; */
	int res;
	DWORD timeout;
	COMMTIMEOUTS TOut;

	timeout = (d_sec*1000) + ((d_usec+999)/1000);

	GetCommTimeouts(fd->handle,&TOut);
	TOut.ReadIntervalTimeout = MAXDWORD;
	TOut.ReadTotalTimeoutMultiplier = 0;
	TOut.ReadTotalTimeoutConstant = timeout;
	SetCommTimeouts(fd->handle,&TOut);

	res = w32_serial_read(fd,buf,buflen,timeout);

	return res;
}
#endif

/* Write up to buflen bytes to fd and return the number of bytes
   written. If no data is available within d_sec + d_usec, return 0.
   On error, a value < 0 is returned (errno indicates error). */
#ifndef WIN32
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
#else
/* Note: currently not implemented de-facto for Win32 */
ssize_t select_write(serial_handler_t *fd, const void *buf, const size_t buflen, const time_t d_sec, const suseconds_t d_usec)
{
	NUT_UNUSED_VARIABLE(fd);
	NUT_UNUSED_VARIABLE(buf);
	NUT_UNUSED_VARIABLE(buflen);
	NUT_UNUSED_VARIABLE(d_sec);
	NUT_UNUSED_VARIABLE(d_usec);
	Serial.printf("WARNING: method %s() is not implemented yet for WIN32", __func__);
	return 0;
}
#endif
