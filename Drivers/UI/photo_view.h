#ifndef __PHOTO_VIEW_H
#define __PHOTO_VIEW_H

#include "lvgl.h"
#include <stdint.h>

#define PHOTO_AREA_W 440U
#define PHOTO_AREA_H 620U

void PhotoView_BindImageObject(lv_obj_t *imgObj);
int PhotoView_ShowRGB565(uint16_t *buf, uint16_t w, uint16_t h);

#endif