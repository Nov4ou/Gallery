#include "photo_view.h"

static lv_obj_t *gPhotoImgObj = NULL;
static lv_image_dsc_t gPhotoDsc;

void PhotoView_BindImageObject(lv_obj_t *imgObj)
{
    gPhotoImgObj = imgObj;
}

int PhotoView_ShowRGB565(uint16_t *buf, uint16_t w, uint16_t h)
{
    if (gPhotoImgObj == NULL || buf == NULL || w == 0 || h == 0)
    {
        return -1;
    }

    gPhotoDsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    gPhotoDsc.header.cf = LV_COLOR_FORMAT_RGB565;
    gPhotoDsc.header.w = w;
    gPhotoDsc.header.h = h;
    gPhotoDsc.data_size = (uint32_t)w * (uint32_t)h * 2U;
    gPhotoDsc.data = (const uint8_t *)buf;

    /* Force LVGL to treat this as updated content */
    lv_image_set_src(gPhotoImgObj, NULL);
    lv_obj_invalidate(gPhotoImgObj);

    lv_image_set_src(gPhotoImgObj, &gPhotoDsc);
    lv_obj_center(gPhotoImgObj);
    lv_obj_invalidate(gPhotoImgObj);

    return 0;
}