#include "lv_port_indev.h"
#include "gt911.h"
#include "lvgl.h"
#include <stdio.h>

static void TouchReadCb(lv_indev_t *indev, lv_indev_data_t *data) {
  GT911_State_t state;
  (void)indev;

  // printf("TouchReadCb entered\r\n");

  if (GT911_ReadState(&state)) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = state.x;
    data->point.y = state.y;
    printf("touch: x=%u y=%u\r\n", state.x, state.y);
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void lv_port_indev_init(void) {
  printf("lv_port_indev_init called\r\n");

  lv_indev_t *indev = lv_indev_create();
  printf("indev=%p\r\n", indev);

  if (indev == NULL) {
    printf("lv_indev_create failed\r\n");
    return;
  }

  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, TouchReadCb);

  printf("lv_port_indev_init done\r\n");
}