#include "common.h"
#include "dstate.h"
#include "extstate.h"

#include "bcmxcp.h"

/* public functions & variables from main.c */
extern const char	*progname, *upsname, *device_name;
extern char		*device_path;
extern int		broken_driver, experimental_driver, do_lock_port, exit_flag;
extern TYPE_FD		upsfd, extrafd;
extern time_t	poll_interval;

/* functions & variables required in each driver */
void upsdrv_initups(void);	/* open connection to UPS, fail if not found */
void upsdrv_initinfo(void);	/* prep data, settings for UPS monitoring */
void upsdrv_updateinfo(void);	/* update state data if possible */
void upsdrv_shutdown(void);	/* make the UPS power off the load */
void upsdrv_help(void);		/* tack on anything useful for the -h text */
void upsdrv_banner(void);	/* print your version information */
void upsdrv_cleanup(void);	/* free any resources before shutdown */

void set_exit_flag(int sig);

/* --- details for the variable/value sharing --- */

/* main calls this driver function - it needs to call addvar */
void upsdrv_makevartable(void);

/* retrieve the value of variable <var> if possible */
char *getval(const char *var);

/* see if <var> has been defined, even if no value has been given to it */
int testvar(const char *var);

/* extended variable table - used for -x defines/flags */
typedef struct vartab_s {
	int	vartype;	/* VAR_* value, below			 */
	char	*var;		/* left side of =, or whole word if none */
	char	*val;		/* right side of = 			 */
	char	*desc;		/* 40 character description for -h text	 */
	int	found;		/* set once encountered, for testvar()	 */
	struct vartab_s	*next;
} vartab_t;

/* flags to define types in the vartab */

#define VAR_FLAG	0x0001	/* argument is a flag (no value needed) */
#define VAR_VALUE	0x0002	/* argument requires a value setting	*/
#define VAR_SENSITIVE	0x0004	/* do not publish in driver.parameter	*/

/* callback from driver - create the table for future -x entries */
void addvar(int vartype, const char *name, const char *desc);

/* functions and data possibly used via libdummy_mockdrv.la for unit-tests */
#ifdef DRIVERS_MAIN_WITHOUT_MAIN
extern vartab_t *vartab_h;
void dparam_setinfo(const char *var, const char *val);
void storeval(const char *var, char *val);
void vartab_free(void);
void setup_signals(void);
#endif /* DRIVERS_MAIN_WITHOUT_MAIN */
