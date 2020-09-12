#include <unistd.h>
#include <stdint.h>

#include "lvgl/lvgl.h"
// New ILI9341 lvgl driver
#include "lv_drivers/display/ILI9341.h"

#include "io.h"

#define DISP_BUF_SIZE (LV_HOR_RES_MAX * LV_VER_RES_MAX)

static lv_color_t buf[DISP_BUF_SIZE];
static lv_disp_buf_t disp_buf;
static lv_disp_drv_t disp_drv;

static lv_obj_t *background = NULL;

int main(int argc, char* argv[])
{
	// LVGL Setup
	lv_init();

	// LCD GPIO and SPI Setup
	io_init(argc, argv);

	// LCD Controller Setup
	ili9341_init();
	ili9341_rotate(90, ILI9341_BGR);

	// LVGL Display Setup (Physical LCD)
	lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);
	lv_disp_drv_init(&disp_drv);
	disp_drv.buffer = &disp_buf;
	disp_drv.flush_cb = ili9341_flush;
	lv_disp_drv_register(&disp_drv);

	// Draw something on the screen
	background = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(background, "Light and Versatile Graphics Library");
	lv_obj_align(background, NULL, LV_ALIGN_CENTER, 0, 0);

	// LED Backlight ON
	io_led(1);

	while(1) {
		lv_task_handler();
		usleep(10000);
	}

	return 0;
}
