#include "main.h"
#include "bcmxcp.h"
#include "bcmxcp_io.h"
#include "serial.h"


static void send_command(unsigned char *command, size_t command_length)
{
	int retry = 0;
	ssize_t sent;
	unsigned char sbuf[1024];

	if (command_length > UCHAR_MAX) {
		upsdebugx (3, "%s: ERROR: command_length too long for the character protocol", __func__);
		return;
	}

	/* Prepare the send buffer */
	sbuf[0] = PW_COMMAND_START_BYTE;
	sbuf[1] = (unsigned char)(command_length);
	memcpy(sbuf+2, command, command_length);
	command_length += 2;

	/* Add checksum */
	sbuf[command_length] = calc_checksum(sbuf);
	command_length += 1;

	upsdebug_hex (3, "send_command", sbuf, command_length);

	while (retry++ < PW_MAX_TRY) {

		if (retry == PW_MAX_TRY) {
			ser_send_char(upsfd, 0x1d); /* last retry is preceded by a ESC.*/
			usleep(250000);
		}

		sent = ser_send_buf(upsfd, sbuf, command_length);

		if (sent < 0) {
			Serial.printf("%s(): error reading from ser_send_buf()");
			return;
		}

		if ((size_t)sent == command_length) {
			return;
		}
	}
}

void send_write_command(unsigned char *command, size_t command_length)
{
	send_command(command, command_length);
}

/* get the answer of a command from the ups. And check that the answer is for this command */
ssize_t get_answer(unsigned char *data, unsigned char command)
{
	unsigned char	my_buf[128]; /* packet has a maximum length of 121+5 bytes */
	ssize_t		res;
	size_t	length, end_length = 0, endblock = 0, start = 0;
	unsigned char	block_number, sequence, pre_sequence = 0;

	while (endblock != 1) {

		do {
			/* Read PW_COMMAND_START_BYTE byte */
			res = ser_get_char(upsfd, my_buf, 1, 0);

			if (res != 1) {
				upsdebugx(1,
					"Receive error (PW_COMMAND_START_BYTE): %" PRIiSIZE ", cmd=%x!!!\n",
					res, command);
				return -1;
			}

			start++;

		} while ((my_buf[0] != PW_COMMAND_START_BYTE) && (start < 128));

		if (start == 128) {
			ser_comm_fail("Receive error (PW_COMMAND_START_BYTE): packet not on start!!%x\n", my_buf[0]);
			return -1;
		}

		/* Read block number byte */
		res = ser_get_char(upsfd, my_buf + 1, 1, 0);

		if (res != 1) {
			ser_comm_fail("Receive error (Block number): %" PRIiSIZE "!!!\n", res);
			return -1;
		}

		block_number = (unsigned char)my_buf[1];

		if (command <= 0x43) {
			if ((command - 0x30) != block_number) {
				ser_comm_fail("Receive error (Request command): %x!!!\n", block_number);
				return -1;
			}
		}

		if (command >= 0x89) {
			if ((command == 0xA0) && (block_number != 0x01)) {
				ser_comm_fail("Receive error (Requested only mode command): %x!!!\n", block_number);
				return -1;
			}

			if ((command != 0xA0) && (block_number != 0x09)) {
				ser_comm_fail("Receive error (Control command): %x!!!\n", block_number);
				return -1;
			}
		}

		/* Read data length byte */
		res = ser_get_char(upsfd, my_buf + 2, 1, 0);

		if (res != 1) {
			ser_comm_fail("Receive error (length): %" PRIiSIZE "!!!\n", res);
			return -1;
		}

		length = (unsigned char)my_buf[2];

		if (length < 1) {
			ser_comm_fail("Receive error (length): packet length %" PRIxSIZE "!!!\n", length);
			return -1;
		}

		/* Read sequence byte */
		res = ser_get_char(upsfd, my_buf + 3, 1, 0);

		if (res != 1) {
			ser_comm_fail("Receive error (sequence): %" PRIiSIZE "!!!\n", res);
			return -1;
		}

		sequence = (unsigned char)my_buf[3];

		if ((sequence & 0x80) == 0x80) {
			endblock = 1;
		}

		if ((sequence & 0x07) != (pre_sequence + 1)) {
			ser_comm_fail("Not the right sequence received %x!!!\n", sequence);
			return -1;
		}

		pre_sequence = sequence;

		/* Try to read all the remaining bytes */
		res = ser_get_buf_len(upsfd, my_buf + 4, length, 1, 0);
		if (res < 0) {
			ser_comm_fail("%s(): ser_get_buf_len() returned error code %" PRIiSIZE, __func__, res);
			return res;
		}

		if ((size_t)res != length) {
			ser_comm_fail("Receive error (data): got %" PRIiSIZE " bytes instead of %" PRIuSIZE "!!!\n", res, length);
			return -1;
		}

		/* Get the checksum byte */
		res = ser_get_char(upsfd, my_buf + (4 + length), 1, 0);

		if (res != 1) {
			ser_comm_fail("Receive error (checksum): %" PRIxSIZE "!!!\n", res);
			return -1;
		}

		/* now we have the whole answer from the ups, we can checksum it */
		if (!checksum_test(my_buf)) {
			ser_comm_fail("checksum error! ");
			return -1;
		}

		memcpy(data+end_length, my_buf + 4, length);
		end_length += length;

	}

	upsdebug_hex (5, "get_answer", data, end_length);

	assert(end_length < SSIZE_MAX);
	return (ssize_t)end_length;
}

static ssize_t command_sequence(unsigned char *command, size_t command_length, unsigned char *answer)
{
	ssize_t	bytes_read;
	int	retry = 0;

	while (retry++ < PW_MAX_TRY) {

		if (retry == PW_MAX_TRY) {
			ser_flush_in(upsfd, "", 0);
		}

		send_write_command(command, command_length);

		bytes_read = get_answer(answer, *command);

		if (bytes_read > 0) {
			return bytes_read;
		}
	}

	return -1;
}

/* Sends a single command (length=1). and get the answer */
ssize_t command_read_sequence(unsigned char command, unsigned char *answer)
{
	ssize_t bytes_read;

	bytes_read = command_sequence(&command, 1, answer);

	if (bytes_read < 1) {
		ser_comm_fail("Error executing command");
	}

	return bytes_read;
}

/* Sends a setup command (length > 1) */
ssize_t command_write_sequence(unsigned char *command, size_t command_length, unsigned	char *answer)
{
	ssize_t bytes_read;

	bytes_read = command_sequence(command, command_length, answer);

	if (bytes_read < 1) {
		ser_comm_fail("Error executing command");
	}

	return bytes_read;
}

void upsdrv_comm_good()
{
	Serial.println("Communication Serial is good");
}

void upsdrv_initups(void)
{
	//define UART3 port
    HardwareSerial USSerial(USART3);
    USSerial.begin(9600);
}
