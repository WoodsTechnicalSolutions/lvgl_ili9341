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
#include "lv_drivers/display/ILI9341.h"
#include "lv_drivers/indev/evdev.h"

#include "io.h"

#define DISP_BUF_SIZE (LV_HOR_RES_MAX * LV_VER_RES_MAX)

static lv_color_t buf[DISP_BUF_SIZE];
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

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

	if (ev->code == LV_EVENT_CLICKED) {
		snprintf(text, sizeof(text), "Button (%d)", ++count);
		lv_label_set_text(button_label, text);
		if (count == UINT8_MAX) {
			count = 0;
		}
	}
}

static void slider_event_cb(lv_event_t *ev)
{
	static char text[4] = { '\0' };

	if (ev->code == LV_EVENT_VALUE_CHANGED) {
		snprintf(text, sizeof(text), "%u", lv_slider_get_value(slider));
		lv_label_set_text(slider_label, text);
	}
}

int main(int argc, char* argv[])
{
	uint8_t count = 0;
	time_t t = time(NULL);

	// LVGL Setup
	lv_init();

	// LCD GPIO and SPI Setup
	io_init(argc, argv);

	// LCD Controller Setup
	ili9341_init();
	ili9341_rotate(90, ILI9341_BGR);

	// Touchscreen
	if (!evdev_set_file(EVDEV_NAME)) {
		io_uninit();
		return -1;
	}

	// LVGL Display Setup (Physical LCD)
	lv_disp_draw_buf_init(&draw_buf, buf, NULL, DISP_BUF_SIZE);
	lv_disp_drv_init(&disp_drv);
	disp_drv.draw_buf = &draw_buf;
	disp_drv.flush_cb = ili9341_flush;
	lv_disp_drv_register(&disp_drv);

	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = evdev_read;
	lv_indev_drv_register(&indev_drv);

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
