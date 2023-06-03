/*
 * bcmxcp_io.h -- header for BCM/XCP IO module
 */

#ifndef BCMXCP_IO__
#define BCMXCP_IO__

#include "main.h" /* for usbdrv_info_t */

void send_write_command(unsigned char *command, size_t command_length);
ssize_t get_answer(unsigned char *data, unsigned char command);
ssize_t command_read_sequence(unsigned char command, unsigned char *data);
ssize_t command_write_sequence(unsigned char *command, size_t command_length, unsigned	char *answer);
void upsdrv_initups(void);
void upsdrv_comm_good(void);
#endif  /* BCMXCP_IO__ */
