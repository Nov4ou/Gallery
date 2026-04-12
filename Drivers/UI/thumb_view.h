#ifndef __THUMB_VIEW_H
#define __THUMB_VIEW_H

#include "lvgl.h"
#include <stdint.h>

void ThumbView_BindImageObject(uint32_t slot, lv_obj_t *imgObj);
void ThumbView_SetThumb(uint32_t slot, uint16_t *buf, uint16_t w, uint16_t h,
                        uint32_t photoIndex);
int ThumbView_GetPhotoIndex(uint32_t slot, uint32_t *photoIndex);

#endif