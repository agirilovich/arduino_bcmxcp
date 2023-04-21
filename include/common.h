/* common.h - prototypes for the common useful functions

   Copyright (C) 2000  Russell Kroll <rkroll@exploits.org>

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


#include "config.h"		/* must be the first header */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>	/* suseconds_t among other things */
#include <sys/stat.h>
#include <stdlib.h>
#include <Arduino.h>

#ifdef HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>	/* for strncasecmp() and strcasecmp() */
#endif

#ifdef HAVE_STRING_H
#include <string.h>	/* for strdup() and many others */
#endif

#include <unistd.h>	/* useconds_t */
#ifndef HAVE_USECONDS_T
# define useconds_t	unsigned long int
#endif
#ifndef HAVE_SUSECONDS_T
/* Note: WIN32 may have this defined as just "long" which should
 * hopefully be identical to the definition below, which we test
 * in our configure script. See also struct timeval fields for a
 * platform, if in doubt.
 */
# define suseconds_t	signed long int
#endif

#include <assert.h>

#include "proto.h"
#include "str.h"

#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/* POSIX requires these, and most but not all systems use same
 * magical numbers for the file descriptors... yep, not all do!
 */
#ifndef STDIN_FILENO
# define STDIN_FILENO  0	/* standard input file descriptor */
#endif
#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1	/* standard output file descriptor */
#endif
#ifndef STDERR_FILENO
# define STDERR_FILENO 2	/* standard error file descriptor */
#endif


/* Type of what file open() and close() use,
 * including pipes for driver-upsd communications: */
# define TYPE_FD int
# define ERROR_FD (-1)
# define VALID_FD(a) (a>=0)

/* Type of what NUT serial/SHUT methods juggle: */
# define TYPE_FD_SER TYPE_FD
# define ERROR_FD_SER ERROR_FD
# define VALID_FD_SER(a) VALID_FD(a)

/* Type of what socket() returns, mostly for networked code: */
# define TYPE_FD_SOCK TYPE_FD
# define ERROR_FD_SOCK ERROR_FD
# define VALID_FD_SOCK(a) VALID_FD(a)

/* Two uppercase letters are more readable than one exclamation */
#define INVALID_FD_SER(a) (!VALID_FD_SER(a))
#define INVALID_FD_SOCK(a) (!VALID_FD_SOCK(a))
#define INVALID_FD(a) (!VALID_FD(a))

extern const char *UPS_VERSION;

/* Use in code to notify the developers and quiesce the compiler that
 * (for this codepath) the argument or variable is unused intentionally.
 * void f(int x) {
 *   NUT_UNUSED_VARIABLE(x);
 *   ...
 * }
 *
 * Note that solutions which mark up function arguments or employ this or
 * that __attribute__ proved not portable enough for wherever NUT builds.
 */
#define NUT_UNUSED_VARIABLE(x) (void)(x)

/** @brief Default timeout (in seconds) for network operations, as used by `upsclient` and `nut-scanner`. */
#define DEFAULT_NETWORK_TIMEOUT		5

/** @brief Default timeout (in seconds) for retrieving the result of a `TRACKING`-enabled operation (e.g. `INSTCMD`, `SET VAR`). */
#define DEFAULT_TRACKING_TIMEOUT	10


int snprintfcat(char *dst, size_t size, const char *fmt, ...)
	__attribute__ ((__format__ (__printf__, 3, 4)));


/* Report CONFIG_FLAGS used for this build of NUT similarly to how
 * upsdebugx(1, ...) would do it, but not limiting the string length
 */
void nut_report_config_flags(void);

extern int nut_debug_level;
extern int nut_log_level;

/* Note: different method signatures instead of TYPE_FD_SER due to "const" */
ssize_t select_read(const int fd, void *buf, const size_t buflen, const time_t d_sec, const suseconds_t d_usec);
ssize_t select_write(const int fd, const void *buf, const size_t buflen, const time_t d_sec, const suseconds_t d_usec);

char * get_libname(const char* base_libname);

/* Buffer sizes used for various functions */
#define SMALLBUF	512
#define LARGEBUF	1024

/** @brief (Minimum) Size that a string must have to hold a UUID4 (i.e. UUID4 length + the terminating null character). */
#define UUID4_LEN	37

/* Provide declarations for getopt() global variables */

#ifdef NEED_GETOPT_H
#include <getopt.h>
#else
#ifdef NEED_GETOPT_DECLS
extern char *optarg;
extern int optind;
#endif /* NEED_GETOPT_DECLS */
#endif /* HAVE_GETOPT_H */


#define SVCNAME TEXT("Network UPS Tools")
#define EVENTLOG_PIPE_NAME TEXT("nut")
#define UPSMON_PIPE_NAME TEXT("upsmon")
#define UPSD_PIPE_NAME TEXT("upsd")

#ifndef HAVE_STRNLEN
size_t strnlen(const char *s, size_t maxlen);
#endif

#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
