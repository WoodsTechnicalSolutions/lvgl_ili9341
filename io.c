/*
 * https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/tools/spi/spidev_test.c
 * https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <gpiod.h>

#include "io.h"

struct gpiod_line *reset_line = NULL;
struct gpiod_line *led_line = NULL;
struct gpiod_line *dc_line = NULL;

static uint8_t spi_bits_per_word = 8;
static uint32_t spi_speed_hz = 32000000;
static uint16_t spi_delay_usecs = 0;

static int spi_fd = -1;

int io_spi_read_array(uint8_t *reg, uint8_t reg_len, uint8_t *rx, uint32_t len)
{
	struct spi_ioc_transfer spi[2];

	memset(spi, 0, sizeof(spi));

	spi[0].tx_buf = (unsigned long)reg;
	spi[0].rx_buf = (unsigned long)NULL;
	spi[0].len = reg_len;
	spi[0].delay_usecs = spi_delay_usecs;
	spi[0].speed_hz = spi_speed_hz;
	spi[0].bits_per_word = spi_bits_per_word;
	spi[1].tx_buf = (unsigned long)NULL;
	spi[1].rx_buf = (unsigned long)rx;
	spi[1].len = len;
	spi[1].delay_usecs = spi_delay_usecs;
	spi[1].speed_hz = spi_speed_hz;
	spi[1].bits_per_word = spi_bits_per_word;

	return ioctl(spi_fd, SPI_IOC_MESSAGE(2), spi);
}

int io_spi_read(uint8_t reg, uint8_t *rx, uint32_t len)
{
	uint8_t data[1] = { reg };

	return io_spi_read_array(data, sizeof(data), rx, len);
}

int io_spi_read_word(uint16_t reg, uint8_t *rx, uint32_t len)
{
	uint8_t data[2] = { (reg & 0x00ff), (reg & 0xff00) >> 8 };

	return io_spi_read_array(data, sizeof(data), rx, len);
}

int io_spi_write_array(uint8_t *tx, uint32_t len)
{
	struct spi_ioc_transfer spi;

	if (!tx) {
		printf("error NULL data\n");
		return -1;
	}

	memset(&spi, 0, sizeof(spi));

	spi.tx_buf = (unsigned long)tx;
	spi.rx_buf = (unsigned long)NULL;
	spi.len = len;
	spi.delay_usecs = spi_delay_usecs;
	spi.speed_hz = spi_speed_hz;
	spi.bits_per_word = spi_bits_per_word;

	return ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
}

int io_spi_write(uint8_t data)
{
	uint8_t tx[1] = { data };

	return io_spi_write_array(tx, sizeof(tx));
}

int io_spi_write_word(uint16_t data)
{
	uint8_t tx[2] = { (data & 0x00ff), (data & 0xff00) >> 8 };

	return io_spi_write_array(tx, sizeof(tx));
}

int io_init(int argc, char *argv[])
{
	int i;
	char chip[16];
	unsigned int offset;

	if (argc <= 2) {
		printf("too few args, try %s /dev/spidevX.Y dc:chip,offset [reset:chip,offset led:chip,offset]\n", argv[0]);
		return -1;
	}

	if ((spi_fd = open(argv[1], O_RDWR | O_CLOEXEC)) < 0) {
		printf("error opening %s\n", argv[1]);
		return -1;
	}

	for (i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "dc:", 3)) {
			sscanf(argv[i], "dc:%[^,],%u", chip, &offset);
			dc_line = gpiod_line_get(chip, offset);
			if (!dc_line) {
				printf("error gpiod_line_get [%s]\n", argv[i]);
				return -1;
			}
			gpiod_line_request_output(dc_line, "ili9341:dc", 0);
		}
		if (!strncmp(argv[i], "led:", 4)) {
			sscanf(argv[i], "led:%[^,],%u", chip, &offset);
			led_line = gpiod_line_get(chip, offset);
			if (!led_line) {
				printf("error gpiod_line_get [%s]\n", argv[i]);
				return -1;
			}
			gpiod_line_request_output(led_line, "ili9341:led", 0);
		}
		if (!strncmp(argv[i], "reset:", 6)) {
			sscanf(argv[i], "reset:%[^,],%u", chip, &offset);
			reset_line = gpiod_line_get(chip, offset);
			if (!reset_line) {
				printf("error gpiod_line_get [%s]\n", argv[i]);
				return -1;
			}
			gpiod_line_request_output(reset_line, "ili9341:reset", 1);
		}
	}

	if (!dc_line) {
		printf("error NULL data/command line\n");
		return -1;
	}

	return 0;
}

void io_uninit(void)
{
	if (dc_line)
		gpiod_line_close_chip(dc_line);

	if (led_line)
		gpiod_line_close_chip(led_line);

	if (reset_line)
		gpiod_line_close_chip(reset_line);
}

int io_dc(int state)
{
	if (dc_line)
		return gpiod_line_set_value(dc_line, state);

	return -1;
}

int io_led(int state)
{
	if (led_line)
		return gpiod_line_set_value(led_line, state);

	return 0;
}

int io_reset(int state)
{
	if (reset_line)
		return gpiod_line_set_value(reset_line, state);

	return 0;
}
