/* dstate.h - Network UPS Tools driver-side state management

   Copyright (C)
	2003	Russell Kroll <rkroll@exploits.org>
	2012-2017	Arnaud Quette <arnaud.quette@free.fr>

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

#include "state.h"

#define DS_LISTEN_BACKLOG 16
#define DS_MAX_READ 256		/* don't read forever from upsd */

#ifndef MAX_STRING_SIZE
#define MAX_STRING_SIZE	128
#endif


	extern	struct	ups_handler	upsh;

	/* asynchronous (nonblocking) Vs synchronous (blocking) I/O
	 * Defaults to nonblocking, for backward compatibility */
	extern	int	do_synchronous;

int dstate_setinfo(const char *var, const char *fmt, ...)
	__attribute__ ((__format__ (__printf__, 2, 3)));
int dstate_addrange(const char *var, const int min, const int max);
void dstate_setflags(const char *var, int flags);
void dstate_setaux(const char *var, long aux);
const char *dstate_getinfo(const char *var);
void dstate_addcmd(const char *cmdname);

void dstate_dataok(void);
void dstate_datastale(void);

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

/* return values for instcmd */
enum {
	STAT_INSTCMD_HANDLED = 0,	/* completed successfully */
	STAT_INSTCMD_UNKNOWN,		/* unspecified error */
	STAT_INSTCMD_INVALID,		/* invalid command */
	STAT_INSTCMD_FAILED		/* command failed */
};

/* return values for setvar */
enum {
	STAT_SET_HANDLED = 0,	/* completed successfully */
	STAT_SET_UNKNOWN,	/* unspecified error */
	STAT_SET_INVALID,	/* not writeable */
	STAT_SET_FAILED		/* writing failed */
};

/* structure for funcs that get called by msg parse routine */
struct ups_handler
{
	int	(*setvar)(const char *, const char *);
	int	(*instcmd)(const char *, const char *);
};
