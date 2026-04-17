#ifndef __THUMB_VIEW_H
#define __THUMB_VIEW_H

#include "lvgl.h"
#include <stdint.h>

int ThumbView_Init(lv_obj_t *parent, uint32_t thumbCount);
void ThumbView_Deinit(void);

lv_obj_t *ThumbView_GetImageObject(uint32_t index);
void ThumbView_SetThumb(uint32_t index, uint16_t *buf, uint16_t w, uint16_t h,
                        uint32_t photoIndex);
int ThumbView_GetPhotoIndex(uint32_t index, uint32_t *photoIndex);

uint32_t ThumbView_GetCount(void);

#endif