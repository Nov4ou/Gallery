#include "jpeg_viewer.h"
#include "ff.h"
#include "tjpgd.h"

typedef struct {
  uint16_t *framebuffer;
  uint16_t fbWidth;
  uint16_t fbHeight;
} JpegDecodeContext_t;

static JpegDecodeContext_t gJpegCtx;

static UINT JpegInput(JDEC *jd, uint8_t *buff, UINT nbyte) {
  FIL *fp = (FIL *)jd->device;
  UINT br = 0;

  if (buff) {
    f_read(fp, buff, nbyte, &br);
  } else {
    /* Skip bytes */
    f_lseek(fp, f_tell(fp) + nbyte);
    br = nbyte;
  }

  return br;
}

int JpegGetInfo(const char *path, uint16_t *w, uint16_t *h) {
  FIL fp;
  JDEC jd;
  JRESULT res;
  static uint8_t work[4096];

  if (w == NULL || h == NULL) {
    return -1;
  }

  if (f_open(&fp, path, FA_READ) != FR_OK) {
    return -2;
  }

  res = jd_prepare(&jd, JpegInput, work, sizeof(work), &fp);
  if (res != JDR_OK) {
    f_close(&fp);
    return -3;
  }

  *w = (uint16_t)jd.width;
  *h = (uint16_t)jd.height;

  f_close(&fp);
  return 0;
}

static int JpegOutput(JDEC *jd, void *bitmap, JRECT *rect) {
  (void)jd;

  uint16_t *src = (uint16_t *)bitmap;
  uint16_t x, y;

  for (y = rect->top; y <= rect->bottom; y++) {
    for (x = rect->left; x <= rect->right; x++) {
      gJpegCtx.framebuffer[y * gJpegCtx.fbWidth + x] = *src++;
    }
  }

  return 1;
}

int JpegDecodeToBufferScaled(const char *path, uint16_t *buffer, uint16_t *w,
                             uint16_t *h, uint8_t scale) {
  FIL fp;
  JDEC jd;
  JRESULT res;
  static uint8_t work[4096];

  if (buffer == NULL || w == NULL || h == NULL) {
    return -1;
  }

  if (f_open(&fp, path, FA_READ) != FR_OK) {
    return -2;
  }

  res = jd_prepare(&jd, JpegInput, work, sizeof(work), &fp);
  if (res != JDR_OK) {
    f_close(&fp);
    return -3;
  }

  *w = (uint16_t)(jd.width >> scale);
  *h = (uint16_t)(jd.height >> scale);
  if (*w == 0)
    *w = 1;
  if (*h == 0)
    *h = 1;

  gJpegCtx.framebuffer = buffer;
  gJpegCtx.fbWidth = (uint16_t)jd.width;
  gJpegCtx.fbHeight = (uint16_t)jd.height;

  res = jd_decomp(&jd, JpegOutput, scale);

  f_close(&fp);

  if (res != JDR_OK) {
    return -4;
  }

  return 0;
}