#include "image_scale.h"

static void FillRGB565(uint16_t *dst, uint16_t w, uint16_t h, uint16_t color) {
  uint32_t i;
  uint32_t total;

  if (dst == 0 || w == 0 || h == 0) {
    return;
  }

  total = (uint32_t)w * (uint32_t)h;

  for (i = 0; i < total; i++) {
    dst[i] = color;
  }
}

void ScaleRGB565Contain(const uint16_t *src, uint16_t srcW, uint16_t srcH,
                        uint16_t *dst, uint16_t dstW, uint16_t dstH,
                        uint16_t bgColor) {
  uint32_t drawW;
  uint32_t drawH;
  uint32_t offX;
  uint32_t offY;
  uint32_t x;
  uint32_t y;

  if (src == 0 || dst == 0 || srcW == 0 || srcH == 0 || dstW == 0 ||
      dstH == 0) {
    return;
  }

  /* Fill destination with background color first */
  FillRGB565(dst, dstW, dstH, bgColor);

  /*
   * Contain:
   * Choose the smaller scale so the whole image fits inside.
   *
   * Compare:
   *   dstW/srcW  ?  dstH/srcH
   * Use integer math:
   *   dstW * srcH  ?  dstH * srcW
   */
  if ((uint32_t)dstW * (uint32_t)srcH < (uint32_t)dstH * (uint32_t)srcW) {
    drawW = dstW;
    drawH = ((uint32_t)srcH * (uint32_t)dstW) / (uint32_t)srcW;
  } else {
    drawH = dstH;
    drawW = ((uint32_t)srcW * (uint32_t)dstH) / (uint32_t)srcH;
  }

  if (drawW == 0)
    drawW = 1;
  if (drawH == 0)
    drawH = 1;

  offX = ((uint32_t)dstW - drawW) / 2U;
  offY = ((uint32_t)dstH - drawH) / 2U;

  for (y = 0; y < drawH; y++) {
    uint32_t srcY = (y * (uint32_t)srcH) / drawH;
    if (srcY >= srcH) {
      srcY = srcH - 1U;
    }

    for (x = 0; x < drawW; x++) {
      uint32_t srcX = (x * (uint32_t)srcW) / drawW;
      if (srcX >= srcW) {
        srcX = srcW - 1U;
      }

      dst[(y + offY) * (uint32_t)dstW + (x + offX)] =
          src[srcY * (uint32_t)srcW + srcX];
    }
  }
}

void ScaleRGB565Cover(const uint16_t *src, uint16_t srcW, uint16_t srcH,
                      uint16_t *dst, uint16_t dstW, uint16_t dstH) {
  uint32_t x;
  uint32_t y;

  if (src == 0 || dst == 0 || srcW == 0 || srcH == 0 || dstW == 0 ||
      dstH == 0) {
    return;
  }

  /*
   * Cover:
   * Choose the larger scale so destination is fully filled.
   *
   * Compare:
   *   dstW/srcW  ?  dstH/srcH
   * Use integer math:
   *   dstW * srcH  ?  dstH * srcW
   */
  if ((uint32_t)dstW * (uint32_t)srcH > (uint32_t)dstH * (uint32_t)srcW) {
    /* Width is tighter, crop vertically */
    uint32_t scaledH = ((uint32_t)srcH * (uint32_t)dstW) / (uint32_t)srcW;
    uint32_t cropY = (scaledH - (uint32_t)dstH) / 2U;

    for (y = 0; y < dstH; y++) {
      uint32_t srcY = ((y + cropY) * (uint32_t)srcW) / (uint32_t)dstW;
      if (srcY >= srcH) {
        srcY = srcH - 1U;
      }

      for (x = 0; x < dstW; x++) {
        uint32_t srcX = (x * (uint32_t)srcW) / (uint32_t)dstW;
        if (srcX >= srcW) {
          srcX = srcW - 1U;
        }

        dst[y * (uint32_t)dstW + x] = src[srcY * (uint32_t)srcW + srcX];
      }
    }
  } else {
    /* Height is tighter, crop horizontally */
    uint32_t scaledW = ((uint32_t)srcW * (uint32_t)dstH) / (uint32_t)srcH;
    uint32_t cropX = (scaledW - (uint32_t)dstW) / 2U;

    for (y = 0; y < dstH; y++) {
      uint32_t srcY = (y * (uint32_t)srcH) / (uint32_t)dstH;
      if (srcY >= srcH) {
        srcY = srcH - 1U;
      }

      for (x = 0; x < dstW; x++) {
        uint32_t srcX = ((x + cropX) * (uint32_t)srcH) / (uint32_t)dstH;
        if (srcX >= srcW) {
          srcX = srcW - 1U;
        }

        dst[y * (uint32_t)dstW + x] = src[srcY * (uint32_t)srcW + srcX];
      }
    }
  }
}