/*
 * Minimal ili9341 LCD panel program using LVGL
 *
 * https://lvgl.io/
 * https://github.com/lvgl/lvgl
 * https://github.com/lvgl/lv_drivers
 * https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/tools/spi/spidev_test.c
 * https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
 *
 * Copyright (C) 2020-2025, Derald D. Woods <woods.technical@gmail.com>
 *
 * This file is made available under the terms of the GNU General Public
 * License version 3.
 */

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "lvgl/lvgl.h"
#include "lvgl/src/drivers/evdev/lv_evdev.h"

#include "io.h"
#include "ili9341.h"

static lv_obj_t *background = NULL;
static lv_obj_t *status = NULL;
static lv_obj_t *button = NULL;
static lv_obj_t *button_label = NULL;
static lv_obj_t *slider = NULL;
static lv_obj_t *slider_label = NULL;

static pthread_t tick_thread;

void *tick_timer(void *arg)
 {
	while (true) {
		usleep(10000);
		lv_tick_inc(10);
	}

	pthread_exit(NULL);
 }

static void btn_event_cb(lv_event_t *ev)
{
	static uint8_t count = 0;
	static char text[32] = { '\0' };

	snprintf(text, sizeof(text), "Button (%d)", ++count);
	lv_label_set_text(button_label, text);
	if (count == UINT8_MAX) {
		count = 0;
	}
}

static void slider_event_cb(lv_event_t *ev)
{
	static char text[4] = { '\0' };

	snprintf(text, sizeof(text), "%u", lv_slider_get_value(slider));
	lv_label_set_text(slider_label, text);
}

int main(int argc, char* argv[])
{
	uint8_t count = 0;
	lv_indev_t *touch = NULL;
	lv_display_t *disp = NULL;
	uint32_t buf_sz;
	uint8_t *buf[2] = { NULL };
	time_t t = time(NULL);

	// LVGL Setup
	lv_init();

	// LCD Controller Setup
	// - HiLetgo ILI9341 240x320 2.8" SPI TFT LCD Touch Panel
	disp = lv_display_create(ILI9341_HORZ_RES, ILI9341_VERT_RES);
	if (!disp)
		return -1;
	ili9341_init(argc, argv, ILI9341_HORZ_RES, ILI9341_VERT_RES);
	ili9341_rotate(90, ILI9341_BGR);
	lv_display_set_flush_cb(disp, ili9341_flush);
	lv_display_set_resolution(disp, ILI9341_HORZ_RES, ILI9341_VERT_RES);
	lv_display_set_physical_resolution(disp, ILI9341_HORZ_RES, ILI9341_VERT_RES);
	lv_display_set_offset(disp, 0, 0);
	buf_sz = ILI9341_HORZ_RES;
	buf_sz *= lv_color_format_get_size(lv_display_get_color_format(disp));
	buf[0] = lv_malloc(buf_sz);
	buf[1] = lv_malloc(buf_sz);
	lv_display_set_buffers(disp, buf[0], buf[1], buf_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);

	// Touchscreen
	touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event1");
	lv_indev_set_display(touch, disp);

	// Set background text on the screen
	background = lv_label_create(lv_scr_act());
	lv_label_set_text(background, "Light and Versatile Graphics Library");
	lv_obj_align(background, LV_ALIGN_CENTER, 0, 75);

	button = lv_btn_create(lv_scr_act());
	lv_obj_set_size(button, 100, 50);
	lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_add_event_cb(button, btn_event_cb, LV_EVENT_CLICKED, NULL);

	button_label = lv_label_create(button);
	lv_label_set_text(button_label, "Button");
	lv_obj_align(button_label, LV_ALIGN_CENTER, 0, 0);

	slider = lv_slider_create(lv_scr_act());
	lv_obj_set_pos(slider, 0, 100);
	lv_obj_set_size(slider, 200, 50);
	lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0);

	slider_label = lv_label_create(lv_scr_act());
	lv_label_set_text(slider_label, "0");
	lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

	// Set status (time) text on the screen
	status = lv_label_create(lv_scr_act());
	lv_label_set_text(status, asctime(localtime(&t)));
	lv_obj_align(status, LV_ALIGN_CENTER, 0, 100);

	if (pthread_create(&tick_thread, NULL, tick_timer, NULL) == -1) {
		fprintf(stderr, "error pthread_create - tick_timer\n");
		return -1;
	}

	// LED Backlight ON
	io_led(1);

	while(1) {
		if (++count == 200) {
			count = 0;
			t = time(NULL);
			lv_obj_clean(status);
			lv_label_set_text(status, asctime(localtime(&t)));
		}
		lv_task_handler();
		usleep(5000);
	}

	return 0;
}
