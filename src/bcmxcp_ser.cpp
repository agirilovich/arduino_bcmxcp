#include "bcmxcp.h"
#include "bcmxcp_io.h"
#include <HardwareSerial.h>

#define SSIZE_MAX ((ssize_t)(-1LL))

HardwareSerial UPSSerial(2);

#define RXD2 16
#define TXD2 17

static void send_command(unsigned char *command, size_t command_length)
{
	int retry = 0;
	ssize_t sent;
	unsigned char sbuf[1024];

	/* Prepare the send buffer */
	sbuf[0] = PW_COMMAND_START_BYTE;
	sbuf[1] = (unsigned char)(command_length);
	memcpy(sbuf+2, command, command_length);
	command_length += 2;

	/* Add checksum */
	sbuf[command_length] = calc_checksum(sbuf);
	command_length += 1;

	while (retry++ < PW_MAX_TRY) {

		if (retry == PW_MAX_TRY) {
			UPSSerial.write(0x1d);
			usleep(250000);
		}

		sent = UPSSerial.write(sbuf, command_length);

		if (sent < 0) {
			Serial.printf("%s(): error reading from UPSSerial write");
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
			res = UPSSerial.readBytes(my_buf, 1);

			if (res != 1) {
				Serial.printf("Receive error (PW_COMMAND_START_BYTE): %",  ", cmd=%x!!!\n", res, command);
				return -1;
			}

			start++;

		} while ((my_buf[0] != PW_COMMAND_START_BYTE) && (start < 128));

		if (start == 128) {
			Serial.printf("Receive error (PW_COMMAND_START_BYTE): packet not on start!!%x\n", my_buf[0]);
			return -1;
		}

		/* Read block number byte */
		res = UPSSerial.readBytes(my_buf + 1, 1);

		if (res != 1) {
			Serial.printf("Receive error (Block number): %", "!!!\n", res);
			return -1;
		}

		block_number = (unsigned char)my_buf[1];

		if (command <= 0x43) {
			if ((command - 0x30) != block_number) {
				Serial.printf("Receive error (Request command): %x!!!\n", block_number);
				return -1;
			}
		}

		if (command >= 0x89) {
			if ((command == 0xA0) && (block_number != 0x01)) {
				Serial.printf("Receive error (Requested only mode command): %x!!!\n", block_number);
				return -1;
			}

			if ((command != 0xA0) && (block_number != 0x09)) {
				Serial.printf("Receive error (Control command): %x!!!\n", block_number);
				return -1;
			}
		}

		/* Read data length byte */
		res = UPSSerial.readBytes(my_buf + 2, 1);

		if (res != 1) {
			Serial.printf("Receive error (length): %", "!!!\n", res);
			return -1;
		}

		length = (unsigned char)my_buf[2];

		if (length < 1) {
			Serial.printf("Receive error (length): packet length % !!!\n", length);
			return -1;
		}

		/* Read sequence byte */
		res = UPSSerial.readBytes(my_buf + 3, 1);

		if (res != 1) {
			Serial.printf("Receive error (sequence): %", "!!!\n", res);
			return -1;
		}

		sequence = (unsigned char)my_buf[3];

		if ((sequence & 0x80) == 0x80) {
			endblock = 1;
		}

		if ((sequence & 0x07) != (pre_sequence + 1)) {
			Serial.printf("Not the right sequence received %x!!!\n", sequence);
			return -1;
		}

		pre_sequence = sequence;

		/* Try to read all the remaining bytes */
		res = UPSSerial.readBytes(my_buf + 4, length);
		if (res < 0) {
			Serial.printf("%s(): Serial read returned error code %", __func__, res);
			return res;
		}

		if ((size_t)res != length) {
			Serial.printf("Receive error (data): got % bytes instead of % !!!\n", res, length);
			return -1;
		}

		/* Get the checksum byte */
		res = UPSSerial.readBytes(my_buf + (4 + length), 1);

		if (res != 1) {
			Serial.printf("Receive error (checksum): % !!!\n", res);
			return -1;
		}

		/* now we have the whole answer from the ups, we can checksum it */
		if (!checksum_test(my_buf)) {
			Serial.printf("checksum error! ");
			return -1;
		}

		memcpy(data+end_length, my_buf + 4, length);
		end_length += length;

	}

	assert(end_length < SSIZE_MAX);
	return (ssize_t)end_length;
}

static ssize_t command_sequence(unsigned char *command, size_t command_length, unsigned char *answer)
{
	ssize_t	bytes_read;
	int	retry = 0;

	while (retry++ < PW_MAX_TRY) {

		if (retry == PW_MAX_TRY) {
			UPSSerial.flush();
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
		Serial.printf("Error executing command");
	}

	return bytes_read;
}

/* Sends a setup command (length > 1) */
ssize_t command_write_sequence(unsigned char *command, size_t command_length, unsigned	char *answer)
{
	ssize_t bytes_read;

	bytes_read = command_sequence(command, command_length, answer);

	if (bytes_read < 1) {
		Serial.printf("Error executing command");
	}

	return bytes_read;
}

void upsdrv_comm_good()
{
	Serial.println("Communication Serial is good");
}

void upsdrv_initups(void)
{
  UPSSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
}
