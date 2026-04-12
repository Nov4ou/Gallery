#ifndef __ALBUM_H
#define __ALBUM_H

#include <stdint.h>

#define EXT_SRAM_BASE ((uint32_t)0x68000000U)

#define PHOTO_DISPLAY_BUF ((uint16_t *)(EXT_SRAM_BASE + 0x000000U))

#define JPG_DECODE_BUF_OFFSET (PHOTO_AREA_W * PHOTO_AREA_H * 2U)
#define JPG_DECODE_BUF ((uint16_t *)(EXT_SRAM_BASE + JPG_DECODE_BUF_OFFSET))

#define JPG_DECODE_MAX_W 640U
#define JPG_DECODE_MAX_H 384U
#define JPG_DECODE_BUF_SIZE_BYTES (JPG_DECODE_MAX_W * JPG_DECODE_MAX_H * 2U)

#define THUMB_W 120U
#define THUMB_H 160U
#define THUMB_PER_PAGE 4U
#define THUMB_BUF_SIZE_BYTES (THUMB_W * THUMB_H * 2U)

#define THUMB_BUF0                                                             \
  ((uint16_t *)((uint8_t *)PHOTO_DISPLAY_BUF + 0 * THUMB_BUF_SIZE_BYTES))
#define THUMB_BUF1                                                             \
  ((uint16_t *)((uint8_t *)PHOTO_DISPLAY_BUF + 1 * THUMB_BUF_SIZE_BYTES))
#define THUMB_BUF2                                                             \
  ((uint16_t *)((uint8_t *)PHOTO_DISPLAY_BUF + 2 * THUMB_BUF_SIZE_BYTES))
#define THUMB_BUF3                                                             \
  ((uint16_t *)((uint8_t *)PHOTO_DISPLAY_BUF + 3 * THUMB_BUF_SIZE_BYTES))

void Album_Init(void);
int Album_ShowByIndex(uint32_t index);
void Album_ShowNext(void);
void Album_ShowPrev(void);

/* Thumb support */
uint32_t Album_GetCount(void);
int Album_LoadThumbPage(uint32_t pageStartIndex);
int Album_OpenFromThumb(uint32_t index);

#endif