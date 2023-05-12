#include <unistd.h>             /* for usleep() and useconds_t, latter also might be via <sys/types.h> */
#include <sys/types.h>
#include <Arduino.h>
#include "str.h"

/* state tree flags */
#define ST_FLAG_RW        0x0001
#define ST_FLAG_STRING    0x0002 /* not STRING implies NUMBER */

/* Buffer sizes used for various functions */
#define SMALLBUF	512
#define LARGEBUF	1024


void send_read_command(unsigned char command);
void send_write_command(unsigned char *command, size_t command_length);
ssize_t get_answer(unsigned char *data, unsigned char command);
ssize_t command_read_sequence(unsigned char command, unsigned char *data);
ssize_t command_write_sequence(unsigned char *command, size_t command_length, unsigned	char *answer);
void upsdrv_reconnect(void);

void upsdrv_initups(unsigned long);	/* open connection to UPS, fail if not found */
void upsdrv_initinfo(void);	/* prep data, settings for UPS monitoring */
void upsdrv_updateinfo(void);	/* update state data if possible */
void upsdrv_shutdown(void);	/* make the UPS power off the load */

/* retrieve the value of variable <var> if possible */
char *getval(const char *var);
