#ifndef __IMAGE_SCALE_H
#define __IMAGE_SCALE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * src    : source RGB565 buffer
 * srcW/H : source width/height
 * dst    : destination RGB565 buffer
 * dstW/H : destination width/height
 * bgColor: background color used for blank area
 *
 * Contain mode:
 * - Keep aspect ratio
 * - Fit the whole image inside destination
 * - May leave borders
 */
void ScaleRGB565Contain(const uint16_t *src, uint16_t srcW, uint16_t srcH,
                        uint16_t *dst, uint16_t dstW, uint16_t dstH,
                        uint16_t bgColor);

/*
 * Cover mode:
 * - Keep aspect ratio
 * - Fill the whole destination
 * - May crop part of the image
 */
void ScaleRGB565Cover(const uint16_t *src, uint16_t srcW, uint16_t srcH,
                      uint16_t *dst, uint16_t dstW, uint16_t dstH);

#ifdef __cplusplus
}
#endif

#endif