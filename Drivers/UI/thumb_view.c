#include "thumb_view.h"

typedef struct {
  lv_obj_t *imgObj;
  lv_image_dsc_t dsc;
  uint32_t photoIndex;
  uint8_t valid;
} ThumbSlot_t;

static ThumbSlot_t gThumbSlots[4];

void ThumbView_BindImageObject(uint32_t slot, lv_obj_t *imgObj) {
  if (slot >= 4U) {
    return;
  }

  gThumbSlots[slot].imgObj = imgObj;
  gThumbSlots[slot].valid = 0;
}

void ThumbView_SetThumb(uint32_t slot, uint16_t *buf, uint16_t w, uint16_t h,
                        uint32_t photoIndex) {
  if (slot >= 4U || gThumbSlots[slot].imgObj == NULL || buf == NULL) {
    return;
  }

  gThumbSlots[slot].dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
  gThumbSlots[slot].dsc.header.cf = LV_COLOR_FORMAT_RGB565;
  gThumbSlots[slot].dsc.header.w = w;
  gThumbSlots[slot].dsc.header.h = h;
  gThumbSlots[slot].dsc.data_size = (uint32_t)w * h * 2U;
  gThumbSlots[slot].dsc.data = (const uint8_t *)buf;

  gThumbSlots[slot].photoIndex = photoIndex;
  gThumbSlots[slot].valid = 1;

  lv_image_set_src(gThumbSlots[slot].imgObj, NULL);
  lv_obj_invalidate(gThumbSlots[slot].imgObj);
  lv_image_set_src(gThumbSlots[slot].imgObj, &gThumbSlots[slot].dsc);
  lv_obj_invalidate(gThumbSlots[slot].imgObj);
}

int ThumbView_GetPhotoIndex(uint32_t slot, uint32_t *photoIndex) {
  if (slot >= 4U || photoIndex == NULL || gThumbSlots[slot].valid == 0) {
    return -1;
  }

  *photoIndex = gThumbSlots[slot].photoIndex;
  return 0;
}