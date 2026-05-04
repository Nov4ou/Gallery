#include "thumb_view.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  lv_obj_t *imgObj;
  lv_image_dsc_t dsc;
  uint32_t photoIndex;
  uint8_t valid;
} ThumbSlot_t;

static ThumbSlot_t *gThumbSlots = NULL;
static uint32_t gThumbCount = 0;
static lv_obj_t *gThumbParent = NULL;

int ThumbView_Init(lv_obj_t *parent, uint32_t thumbCount) {
  uint32_t i;

  if (parent == NULL || thumbCount == 0U) {
    return -1;
  }

  ThumbView_Deinit();

  gThumbSlots = (ThumbSlot_t *)calloc(thumbCount, sizeof(ThumbSlot_t));
  if (gThumbSlots == NULL) {
    return -2;
  }

  gThumbCount = thumbCount;
  gThumbParent = parent;

  for (i = 0; i < gThumbCount; i++) {
    gThumbSlots[i].imgObj = lv_image_create(gThumbParent);
    gThumbSlots[i].valid = 0;

    /* You can adjust size/style here */
    lv_obj_set_size(gThumbSlots[i].imgObj, 80, 80);
  }

  return 0;
}

void ThumbView_Deinit(void) {
  uint32_t i;

  if (gThumbSlots != NULL) {
    for (i = 0; i < gThumbCount; i++) {
      if (gThumbSlots[i].imgObj != NULL) {
        lv_obj_del(gThumbSlots[i].imgObj);
        gThumbSlots[i].imgObj = NULL;
      }
    }

    free(gThumbSlots);
    gThumbSlots = NULL;
  }

  gThumbCount = 0;
  gThumbParent = NULL;
}

lv_obj_t *ThumbView_GetImageObject(uint32_t index) {
  if (gThumbSlots == NULL || index >= gThumbCount) {
    return NULL;
  }

  return gThumbSlots[index].imgObj;
}

uint32_t ThumbView_GetCount(void) { return gThumbCount; }

void ThumbView_SetThumb(uint32_t index, uint16_t *buf, uint16_t w, uint16_t h,
                        uint32_t photoIndex) {
  if (gThumbSlots == NULL || index >= gThumbCount ||
      gThumbSlots[index].imgObj == NULL || buf == NULL) {
    return;
  }

  gThumbSlots[index].dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
  gThumbSlots[index].dsc.header.cf = LV_COLOR_FORMAT_RGB565;
  gThumbSlots[index].dsc.header.w = w;
  gThumbSlots[index].dsc.header.h = h;
  gThumbSlots[index].dsc.data_size = (uint32_t)w * h * 2U;
  gThumbSlots[index].dsc.data = (const uint8_t *)buf;

  gThumbSlots[index].photoIndex = photoIndex;
  gThumbSlots[index].valid = 1;

  lv_image_set_src(gThumbSlots[index].imgObj, NULL);
  lv_obj_invalidate(gThumbSlots[index].imgObj);
  lv_image_set_src(gThumbSlots[index].imgObj, &gThumbSlots[index].dsc);
  lv_obj_invalidate(gThumbSlots[index].imgObj);
}

int ThumbView_GetPhotoIndex(uint32_t index, uint32_t *photoIndex) {
  if (gThumbSlots == NULL || index >= gThumbCount || photoIndex == NULL ||
      gThumbSlots[index].valid == 0) {
    return -1;
  }

  *photoIndex = gThumbSlots[index].photoIndex;
  return 0;
}