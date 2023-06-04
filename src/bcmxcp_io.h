#include <Arduino.h>
#include <sys/types.h>
#include "state.h"

void send_write_command(unsigned char *command, size_t command_length);
ssize_t get_answer(unsigned char *data, unsigned char command);
ssize_t command_read_sequence(unsigned char command, unsigned char *data);
ssize_t command_write_sequence(unsigned char *command, size_t command_length, unsigned	char *answer);
void upsdrv_initups(void);
void upsdrv_comm_good(void);

void upsdrv_initinfo(void);	/* prep data, settings for UPS monitoring */
void upsdrv_updateinfo(void);	/* update state data if possible */
void upsdrv_shutdown(void);	/* make the UPS power off the load */
