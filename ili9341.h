#ifndef ILI9341_H
#define ILI9341_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "lvgl/lvgl.h"

#define ILI9341_HORZ_RES 240
#define ILI9341_VERT_RES 320

#define ILI9341_BGR true
#define ILI9341_RGB false

void ili9341_init(int argc, char *argv[], int32_t lcd_h, int32_t lcd_v);
void ili9341_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void ili9341_rotate(int degrees, bool bgr);

void ili9341_write(int mode, uint8_t data);
void ili9341_write_array(int mode, uint8_t *data, uint16_t len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ILI9341_H */
