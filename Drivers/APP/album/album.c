#include "album.h"
#include "ff.h"
#include "image_scale.h"
#include "jpeg_viewer.h"
#include "photo_view.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

char gPhotoList[MAX_PHOTO_COUNT][MAX_PHOTO_PATH];
uint32_t gPhotoCount = 0;
uint32_t gCurrentPhotoIndex = 0;

static uint32_t gCategoryPhotoIndex[MAX_PHOTO_COUNT];
static uint32_t gCategoryPhotoCount = 0;
static uint32_t gCurrentCategoryIndex = 0;
static AlbumCategory_t gCurrentCategory = AlbumCategoryAll;

static int StrCaseStr(const char *haystack, const char *needle);
static int Album_MatchesCategory(const char *fileName, AlbumCategory_t category);
static void Album_RebuildCategoryList(uint32_t preferredPhotoIndex);

void Album_Init(void) {
  Album_RebuildCategoryList(gCurrentPhotoIndex);
  Album_ShowCurrent();
}

static int StrCaseCmp(const char *a, const char *b) {
  while (*a && *b) {
    char ca = (char)tolower((unsigned char)*a);
    char cb = (char)tolower((unsigned char)*b);

    if (ca != cb) {
      return (int)(unsigned char)ca - (int)(unsigned char)cb;
    }

    a++;
    b++;
  }

  return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

static int StrCaseStr(const char *haystack, const char *needle) {
  size_t needleLen;

  if (haystack == NULL || needle == NULL) {
    return 0;
  }

  needleLen = strlen(needle);
  if (needleLen == 0U) {
    return 1;
  }

  while (*haystack != '\0') {
    size_t i;

    for (i = 0; i < needleLen; i++) {
      char hc = (char)tolower((unsigned char)haystack[i]);
      char nc = (char)tolower((unsigned char)needle[i]);

      if (haystack[i] == '\0' || hc != nc) {
        break;
      }
    }

    if (i == needleLen) {
      return 1;
    }

    haystack++;
  }

  return 0;
}

static int Album_MatchesCategory(const char *fileName,
                                 AlbumCategory_t category) {
  switch (category) {
  case AlbumCategoryAll:
    return 1;
  case AlbumCategoryBuilding:
    return StrCaseStr(fileName, "bld");
  case AlbumCategoryScenery:
    return StrCaseStr(fileName, "scn");
  case AlbumCategoryGame:
    return StrCaseStr(fileName, "gam");
  case AlbumCategoryPerson:
    return StrCaseStr(fileName, "per");
  case AlbumCategoryAnimal:
    return StrCaseStr(fileName, "ani");
  default:
    return 0;
  }
}

static void Album_RebuildCategoryList(uint32_t preferredPhotoIndex) {
  uint32_t i;

  gCategoryPhotoCount = 0;
  gCurrentCategoryIndex = 0;
  memset(gCategoryPhotoIndex, 0, sizeof(gCategoryPhotoIndex));

  for (i = 0; i < gPhotoCount; i++) {
    if (Album_MatchesCategory(gPhotoList[i], gCurrentCategory)) {
      if (gCategoryPhotoCount < MAX_PHOTO_COUNT) {
        gCategoryPhotoIndex[gCategoryPhotoCount] = i;
        if (i == preferredPhotoIndex) {
          gCurrentCategoryIndex = gCategoryPhotoCount;
        }
        gCategoryPhotoCount++;
      }
    }
  }

  if (gCategoryPhotoCount == 0U) {
    gCurrentPhotoIndex = 0U;
    return;
  }

  if (gCurrentCategoryIndex >= gCategoryPhotoCount) {
    gCurrentCategoryIndex = 0U;
  }

  gCurrentPhotoIndex = gCategoryPhotoIndex[gCurrentCategoryIndex];
}

int Album_IsJpgFile(const char *fileName) {
  const char *ext;

  if (fileName == NULL) {
    return 0;
  }

  ext = strrchr(fileName, '.');
  if (ext == NULL) {
    return 0;
  }

  if (StrCaseCmp(ext, ".jpg") == 0) {
    return 1;
  }

  if (StrCaseCmp(ext, ".jpeg") == 0) {
    return 1;
  }

  return 0;
}

int Album_ScanPhotos(const char *dirPath) {
  DIR dir;
  FILINFO fno;
  FRESULT res;
  uint32_t oldIndex;

  if (dirPath == NULL) {
    return -1;
  }

  oldIndex = gCurrentPhotoIndex;

  gPhotoCount = 0;
  memset(gPhotoList, 0, sizeof(gPhotoList));

  res = f_opendir(&dir, dirPath);
  if (res != FR_OK) {
    printf("f_opendir failed: %d\r\n", res);
    return -2;
  }

  while (1) {
    res = f_readdir(&dir, &fno);
    if (res != FR_OK) {
      printf("f_readdir failed: %d\r\n", res);
      f_closedir(&dir);
      return -3;
    }

    if (fno.fname[0] == '\0') {
      break;
    }

    if (fno.fattrib & AM_DIR) {
      continue;
    }

    if (Album_IsJpgFile(fno.fname)) {
      if (gPhotoCount < MAX_PHOTO_COUNT) {
        snprintf(gPhotoList[gPhotoCount], MAX_PHOTO_PATH, "%s%s", dirPath,
                 fno.fname);

        printf("Found photo[%lu]: %s\r\n", (unsigned long)gPhotoCount,
               gPhotoList[gPhotoCount]);

        gPhotoCount++;
      } else {
        printf("Photo list full\r\n");
        break;
      }
    }
  }

  f_closedir(&dir);

  Album_RebuildCategoryList(oldIndex);

  printf("Album_ScanPhotos done, count=%lu, category=%d, categoryCount=%lu, "
         "current=%lu\r\n",
         (unsigned long)gPhotoCount, (int)gCurrentCategory,
         (unsigned long)gCategoryPhotoCount, (unsigned long)gCurrentPhotoIndex);

  return 0;
}

int Album_ShowByIndex(uint32_t index) {
  uint16_t srcW, srcH;
  uint16_t jpgW, jpgH;
  uint8_t scale = 0;
  int ret;
  uint16_t *jpgBuf = JPG_DECODE_BUF;
  uint16_t *photoBuf = PHOTO_DISPLAY_BUF;

  if (index >= gPhotoCount) {
    return -1;
  }

  /* Step 1: read original JPEG size first */
  ret = JpegGetInfo(gPhotoList[index], &srcW, &srcH);
  if (ret != 0) {
    printf("Album_ShowByIndex: get info failed, index=%lu, ret=%d, path=%s\r\n",
           (unsigned long)index, ret, gPhotoList[index]);
    return -2;
  }

  /* Step 2: choose decode scale automatically */
  scale = 0;
  while ((((uint32_t)srcW >> scale) > JPG_DECODE_MAX_W ||
          ((uint32_t)srcH >> scale) > JPG_DECODE_MAX_H) &&
         scale < 3U) {
    scale++;
  }

  /* Step 3: decode JPEG with selected scale */
  ret =
      JpegDecodeToBufferScaled(gPhotoList[index], jpgBuf, &jpgW, &jpgH, scale);
  printf("Album_ShowByIndex: index=%lu, ret=%d, src=%ux%u, scale=%u, "
         "decode=%ux%u, path=%s\r\n",
         (unsigned long)index, ret, srcW, srcH, scale, jpgW, jpgH,
         gPhotoList[index]);

  if (ret != 0) {
    return -3;
  }

  /* Step 4: safety check for decode buffer */
  if (((uint32_t)jpgW * (uint32_t)jpgH * 2U) > JPG_DECODE_BUF_SIZE_BYTES) {
    printf("Album_ShowByIndex: decoded image too large for decode buffer, "
           "%ux%u\r\n",
           jpgW, jpgH);
    return -4;
  }

  /* Step 5: scale to photo area and show */
  ScaleRGB565Contain(jpgBuf, jpgW, jpgH, photoBuf, PHOTO_AREA_W, PHOTO_AREA_H,
                     0x0000);

  PhotoView_ShowRGB565(photoBuf, PHOTO_AREA_W, PHOTO_AREA_H);

  gCurrentPhotoIndex = index;
  for (uint32_t i = 0; i < gCategoryPhotoCount; i++) {
    if (gCategoryPhotoIndex[i] == index) {
      gCurrentCategoryIndex = i;
      break;
    }
  }

  return 0;
}

int Album_ShowCurrent(void) {
  if (gCategoryPhotoCount == 0U) {
    printf("Album_ShowCurrent: no photo in category %d\r\n",
           (int)gCurrentCategory);
    return -1;
  }

  return Album_ShowByIndex(gCategoryPhotoIndex[gCurrentCategoryIndex]);
}

void Album_ShowNext(void) {
  if (gCategoryPhotoCount == 0U) {
    printf("Album_ShowNext: no photo in category %d\r\n",
           (int)gCurrentCategory);
    return;
  }

  gCurrentCategoryIndex++;
  if (gCurrentCategoryIndex >= gCategoryPhotoCount) {
    gCurrentCategoryIndex = 0U;
  }

  Album_ShowByIndex(gCategoryPhotoIndex[gCurrentCategoryIndex]);
}

void Album_ShowPrev(void) {
  if (gCategoryPhotoCount == 0U) {
    printf("Album_ShowPrev: no photo in category %d\r\n",
           (int)gCurrentCategory);
    return;
  }

  if (gCurrentCategoryIndex == 0U) {
    gCurrentCategoryIndex = gCategoryPhotoCount - 1U;
  } else {
    gCurrentCategoryIndex--;
  }

  Album_ShowByIndex(gCategoryPhotoIndex[gCurrentCategoryIndex]);
}

uint32_t Album_SetCategory(AlbumCategory_t category) {
  if (category > AlbumCategoryAnimal) {
    return gCategoryPhotoCount;
  }

  gCurrentCategory = category;
  Album_RebuildCategoryList(gCurrentPhotoIndex);

  printf("Album_SetCategory: category=%d count=%lu\r\n", (int)gCurrentCategory,
         (unsigned long)gCategoryPhotoCount);

  return gCategoryPhotoCount;
}

AlbumCategory_t Album_GetCategory(void) { return gCurrentCategory; }

uint32_t Album_GetCategoryCount(void) { return gCategoryPhotoCount; }

uint32_t Album_GetCount(void) { return gPhotoCount; }
