#ifndef __ALBUM_H
#define __ALBUM_H

#include <stdint.h>

#define EXT_SRAM_BASE ((uint32_t)0x68000000U)

#define MAX_PHOTO_COUNT 64U
#define MAX_PHOTO_PATH 64U

#define PHOTO_DISPLAY_BUF ((uint16_t *)(EXT_SRAM_BASE + 0x000000U))

#define JPG_DECODE_BUF_OFFSET (PHOTO_AREA_W * PHOTO_AREA_H * 2U)
#define JPG_DECODE_BUF ((uint16_t *)(EXT_SRAM_BASE + JPG_DECODE_BUF_OFFSET))

#define JPG_DECODE_MAX_W 640U
#define JPG_DECODE_MAX_H 384U
#define JPG_DECODE_BUF_SIZE_BYTES (JPG_DECODE_MAX_W * JPG_DECODE_MAX_H * 2U)

extern char gPhotoList[MAX_PHOTO_COUNT][MAX_PHOTO_PATH];
extern uint32_t gPhotoCount;
extern uint32_t gCurrentPhotoIndex;

typedef enum {
  AlbumCategoryAll = 0,
  AlbumCategoryBuilding,
  AlbumCategoryScenery,
  AlbumCategoryGame,
  AlbumCategoryPerson,
  AlbumCategoryAnimal,
} AlbumCategory_t;

void Album_Init(void);
int Album_ScanPhotos(const char *dirPath);
int Album_IsJpgFile(const char *fileName);
int Album_ShowByIndex(uint32_t index);
int Album_ShowCurrent(void);
void Album_ShowNext(void);
void Album_ShowPrev(void);
uint32_t Album_SetCategory(AlbumCategory_t category);
AlbumCategory_t Album_GetCategory(void);
uint32_t Album_GetCategoryCount(void);
uint32_t Album_GetCount(void);
AlbumCategory_t Album_GetCategoryFromFileName(const char *fileName);
int Album_OpenUploadedPhoto(const char *fileName);

#endif
