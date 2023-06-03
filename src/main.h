#ifndef NUT_MAIN_H_SEEN
#define NUT_MAIN_H_SEEN

#include "dstate.h"

extern TYPE_FD		upsfd, extrafd;

/* functions & variables required in each driver */
void upsdrv_initups(void);	/* open connection to UPS, fail if not found */
void upsdrv_initinfo(void);	/* prep data, settings for UPS monitoring */
void upsdrv_updateinfo(void);	/* update state data if possible */
void upsdrv_shutdown(void);	/* make the UPS power off the load */

/* retrieve the value of variable <var> if possible */
char *getval(const char *var);

#endif /* NUT_MAIN_H_SEEN */