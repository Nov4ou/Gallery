#ifndef __JPEG_VIEWER_H
#define __JPEG_VIEWER_H

#include "main.h"
#include <stdint.h>

int JpegGetInfo(const char *path, uint16_t *w, uint16_t *h);
int JpegDecodeToBufferScaled(const char *path, uint16_t *buffer, uint16_t *w,
                             uint16_t *h, uint8_t scale);

#endif