/*
 * io.h - SPI and GPIO I/O routine declarations
 *
 * Copyright (C) 2020-2025, Derald D. Woods <woods.technical@gmail.com>
 *
 * This file is made available under the terms of the GNU General Public
 * License version 3.
 */

#ifndef IO_H
#define IO_H

int io_init(int argc, char *argv[]);
void io_uninit(void);

int io_spi_read(uint8_t reg, uint8_t *rx, uint32_t len);
int io_spi_read_word(uint16_t reg, uint8_t *rx, uint32_t len);
int io_spi_read_array(uint8_t *reg, uint8_t reg_len, uint8_t *rx, uint32_t len);
int io_spi_write(uint8_t data);
int io_spi_write_word(uint16_t data);
int io_spi_write_array(uint8_t *tx, uint32_t len);

int io_dc(int state);
int io_led(int state);
int io_reset(int state);

#endif
