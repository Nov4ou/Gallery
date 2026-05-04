#include "lv_port_disp.h"
#include "lcd.h"
#include <stdint.h>

#define MY_DISP_HOR_RES 480
#define MY_DISP_VER_RES 800

static uint16_t lvgl_buf_1[MY_DISP_HOR_RES * 20];

static void my_flush_cb(lv_display_t *disp, const lv_area_t *area,
                        uint8_t *px_map) {
  uint16_t x, y;
  uint16_t *color_p = (uint16_t *)px_map;

  LCD_SetWindow((uint16_t)area->x1, (uint16_t)area->y1, (uint16_t)area->x2,
                (uint16_t)area->y2);

  for (y = (uint16_t)area->y1; y <= (uint16_t)area->y2; y++) {
    for (x = (uint16_t)area->x1; x <= (uint16_t)area->x2; x++) {
      LCD_WriteColor(*color_p++);
    }
  }

  lv_display_flush_ready(disp);
}

void lv_port_disp_init(void) {
  lv_display_t *disp;
  disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);

  lv_display_set_buffers(disp, lvgl_buf_1, NULL, sizeof(lvgl_buf_1),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_display_set_flush_cb(disp, my_flush_cb);
}