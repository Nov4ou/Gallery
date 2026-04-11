#include "bmp.h"
#include "lcd.h"
#include <string.h>

/*
 * For 24-bit BMP:
 * Row size = ((width * 3) + 3) & ~3
 * For 480 pixels:
 * Row size = 1440, already aligned
 */
#define BMP_MAX_WIDTH 480U
#define BMP_MAX_ROW_BYTES (((BMP_MAX_WIDTH * 3U) + 3U) & ~3U)

static uint8_t bmpRowBuf[BMP_MAX_ROW_BYTES];

static uint16_t BMP_ReadLE16(const uint8_t *p) {
  return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t BMP_ReadLE32(const uint8_t *p) {
  return (uint32_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                    ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

static int32_t BMP_ReadLE32_Signed(const uint8_t *p) {
  return (int32_t)BMP_ReadLE32(p);
}

static uint16_t BMP_BGR888_To_RGB565(uint8_t b, uint8_t g, uint8_t r) {
  return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

int BMP_ShowFromSD(const char *path, uint16_t startX, uint16_t startY) {
  FIL file;
  FRESULT res;
  UINT br;

  uint8_t header[54];
  uint32_t dataOffset;
  int32_t width;
  int32_t height;
  uint16_t bitCount;
  uint32_t compression;
  uint32_t rowSize;
  uint16_t x;
  uint16_t y;

  res = f_open(&file, path, FA_READ);
  if (res != FR_OK) {
    return -1;
  }

  res = f_read(&file, header, sizeof(header), &br);
  if (res != FR_OK || br != sizeof(header)) {
    f_close(&file);
    return -2;
  }

  /* Check signature "BM" */
  if (BMP_ReadLE16(&header[0]) != 0x4D42U) {
    f_close(&file);
    return -3;
  }

  dataOffset = BMP_ReadLE32(&header[10]);
  width = BMP_ReadLE32_Signed(&header[18]);
  height = BMP_ReadLE32_Signed(&header[22]);
  bitCount = BMP_ReadLE16(&header[28]);
  compression = BMP_ReadLE32(&header[30]);

  if (width <= 0 || height == 0) {
    f_close(&file);
    return -4;
  }

  if (bitCount != 24U) {
    f_close(&file);
    return -5;
  }

  if (compression != 0U) {
    f_close(&file);
    return -6;
  }

  if ((uint32_t)width > BMP_MAX_WIDTH) {
    f_close(&file);
    return -7;
  }

  if ((startX + (uint16_t)width) > LCD_WIDTH ||
      (startY + (uint16_t)((height > 0) ? height : -height)) > LCD_HEIGHT) {
    f_close(&file);
    return -8;
  }

  rowSize = ((uint32_t)width * 3U + 3U) & ~3U;

  if (height > 0) {
    /* Bottom-up BMP */
    for (y = 0; y < (uint16_t)height; y++) {
      uint32_t filePos = dataOffset + (uint32_t)(height - 1 - y) * rowSize;

      res = f_lseek(&file, filePos);
      if (res != FR_OK) {
        f_close(&file);
        return -9;
      }

      res = f_read(&file, bmpRowBuf, rowSize, &br);
      if (res != FR_OK || br != rowSize) {
        f_close(&file);
        return -10;
      }

      LCD_SetWindow(startX, startY + y, startX + (uint16_t)width - 1U,
                    startY + y);

      for (x = 0; x < (uint16_t)width; x++) {
        uint8_t b = bmpRowBuf[x * 3U + 0U];
        uint8_t g = bmpRowBuf[x * 3U + 1U];
        uint8_t r = bmpRowBuf[x * 3U + 2U];

        LCD_WriteColor(BMP_BGR888_To_RGB565(b, g, r));
      }
    }
  } else {
    /* Top-down BMP */
    height = -height;

    for (y = 0; y < (uint16_t)height; y++) {
      uint32_t filePos = dataOffset + (uint32_t)y * rowSize;

      res = f_lseek(&file, filePos);
      if (res != FR_OK) {
        f_close(&file);
        return -11;
      }

      res = f_read(&file, bmpRowBuf, rowSize, &br);
      if (res != FR_OK || br != rowSize) {
        f_close(&file);
        return -12;
      }

      LCD_SetWindow(startX, startY + y, startX + (uint16_t)width - 1U,
                    startY + y);

      for (x = 0; x < (uint16_t)width; x++) {
        uint8_t b = bmpRowBuf[x * 3U + 0U];
        uint8_t g = bmpRowBuf[x * 3U + 1U];
        uint8_t r = bmpRowBuf[x * 3U + 2U];

        LCD_WriteColor(BMP_BGR888_To_RGB565(b, g, r));
      }
    }
  }

  f_close(&file);
  return 0;
}