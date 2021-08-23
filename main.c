#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "lvgl/lvgl.h"
// New ILI9341 lvgl driver
#include "lv_drivers/display/ILI9341.h"

#include "io.h"

#define DISP_BUF_SIZE (LV_HOR_RES_MAX * LV_VER_RES_MAX)

static lv_color_t buf[DISP_BUF_SIZE];
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

static lv_obj_t *background = NULL;
static lv_obj_t *status = NULL;

static pthread_t tick_thread;

void *tick_timer(void *arg)
 {
	while (true) {
		usleep(10000);
		lv_tick_inc(10);
	}

	pthread_exit(NULL);
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

	// LVGL Display Setup (Physical LCD)
	lv_disp_draw_buf_init(&draw_buf, buf, NULL, DISP_BUF_SIZE);
	lv_disp_drv_init(&disp_drv);
	disp_drv.draw_buf = &draw_buf;
	disp_drv.flush_cb = ili9341_flush;
	lv_disp_drv_register(&disp_drv);

	// Set background text on the screen
	background = lv_label_create(lv_scr_act());
	lv_label_set_text(background, "Light and Versatile Graphics Library");
	lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);

	// Set status (time) text on the screen
	status = lv_label_create(lv_scr_act());
	lv_label_set_text(status, asctime(localtime(&t)));
	lv_obj_align(status, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

	if (pthread_create(&tick_thread, NULL, tick_timer, NULL) == -1) {
		fprintf(stderr, "error pthread_create - tick_timer\n");
		return -1;
	}

	// LED Backlight ON
	io_led(1);

	while(1) {
		if (++count == 10) {
			count = 0;
			t = time(NULL);
			lv_obj_clean(status);
			lv_label_set_text(status, asctime(localtime(&t)));
		}
		lv_task_handler();
		usleep(100000);
	}

	return 0;
}
