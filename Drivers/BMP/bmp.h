#ifndef __BMP_H
#define __BMP_H

#include "ff.h"
#include "main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int BMP_ShowFromSD(const char *path, uint16_t startX, uint16_t startY);
#ifdef __cplusplus
}
#endif

#endif