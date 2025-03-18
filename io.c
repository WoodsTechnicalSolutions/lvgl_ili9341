/*
 * io.c - SPI and GPIO I/O routine implementation
 *
 * Copyright (C) 2020-2025, Derald D. Woods <woods.technical@gmail.com>
 *
 * This file is made available under the terms of the GNU General Public
 * License version 3.
 */

#include <unistd.h>
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

struct gpiod_line_request *reset_line_request = NULL;
struct gpiod_line_request *led_line_request = NULL;
struct gpiod_line_request *dc_line_request = NULL;

static uint8_t spi_bits_per_word = 8;
static uint32_t spi_speed_hz = 32000000;
static uint16_t spi_delay_usecs = 0;

static int spi_fd = -1;

// https://web.git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/tree/examples/toggle_line_value.c
struct gpiod_line_request *request_output_line(const char *chip_path,
		unsigned int offset, enum gpiod_line_value value, const char *consumer)
{
	struct gpiod_request_config *req_cfg = NULL;
	struct gpiod_line_request *request = NULL;
	struct gpiod_line_settings *settings;
	struct gpiod_line_config *line_cfg;
	struct gpiod_chip *chip;
	int rc;

	chip = gpiod_chip_open(chip_path);
	if (!chip)
		return NULL;

	settings = gpiod_line_settings_new();
	if (!settings)
		goto close_chip;

	gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
	gpiod_line_settings_set_output_value(settings, value);

	line_cfg = gpiod_line_config_new();
	if (!line_cfg)
		goto free_settings;

	rc = gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);
	if (rc)
		goto free_line_config;

	if (consumer) {
		req_cfg = gpiod_request_config_new();
		if (!req_cfg)
			goto free_line_config;

		gpiod_request_config_set_consumer(req_cfg, consumer);
	}

	request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

	gpiod_request_config_free(req_cfg);

free_line_config:
	gpiod_line_config_free(line_cfg);

free_settings:
	gpiod_line_settings_free(settings);

close_chip:
	gpiod_chip_close(chip);

	return request;
}

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

int io_spi_write_array(const uint8_t *tx, uint32_t len)
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
	char chip_num[4];
	char chip_path[20];
	unsigned int offset;
	enum gpiod_line_value value = GPIOD_LINE_VALUE_ACTIVE;

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
			sscanf(argv[i], "dc:%[^,],%u", chip_num, &offset);
			snprintf(chip_path, sizeof(chip_path), "/dev/gpiochip%s", chip_num);
			dc_line_request = request_output_line(chip_path, offset, value, "ili9341:dc");
			if (!dc_line_request) {
				printf("error request_output_line [%s]\n", argv[i]);
				return -1;
			}
		}
		if (!strncmp(argv[i], "led:", 4)) {
			sscanf(argv[i], "led:%[^,],%u", chip_num, &offset);
			snprintf(chip_path, sizeof(chip_path), "/dev/gpiochip%s", chip_num);
			led_line_request = request_output_line(chip_path, offset, value, "ili9341:led");
			if (!led_line_request) {
				printf("error request_output_line [%s]\n", argv[i]);
				return -1;
			}
		}
		if (!strncmp(argv[i], "reset:", 6)) {
			sscanf(argv[i], "reset:%[^,],%u", chip_num, &offset);
			snprintf(chip_path, sizeof(chip_path), "/dev/gpiochip%s", chip_num);
			reset_line_request = request_output_line(chip_path, offset, value, "ili9341:reset");
			if (!reset_line_request) {
				printf("error request_output_line [%s]\n", argv[i]);
				return -1;
			}
		}
	}

	if (!dc_line_request) {
		printf("error NULL data/command line\n");
		return -1;
	}

	return 0;
}

void io_uninit(void)
{
	if (dc_line_request)
		gpiod_line_request_release(dc_line_request);

	if (led_line_request)
		gpiod_line_request_release(led_line_request);

	if (reset_line_request)
		gpiod_line_request_release(reset_line_request);
}

int io_dc(int state)
{
	unsigned int offset;

	if (dc_line_request) {
		gpiod_line_request_get_requested_offsets(dc_line_request, &offset, 1);
		return gpiod_line_request_set_value(dc_line_request, offset,
				state == 1 ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
	}

	return -1;
}

int io_led(int state)
{
	unsigned int offset;

	if (led_line_request) {
		gpiod_line_request_get_requested_offsets(led_line_request, &offset, 1);
		return gpiod_line_request_set_value(led_line_request, offset,
				state == 1 ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
	}

	return 0;
}

int io_reset(int state)
{
	unsigned int offset;

	if (reset_line_request) {
		gpiod_line_request_get_requested_offsets(reset_line_request, &offset, 1);
		return gpiod_line_request_set_value(reset_line_request, offset,
				state == 1 ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
	}

	return 0;
}

void io_delay_us(uint32_t us)
{
	usleep(us);
}

void io_delay_ms(uint32_t ms)
{
	usleep(ms * 1000);
}
