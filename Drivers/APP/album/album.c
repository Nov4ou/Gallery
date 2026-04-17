#include "album.h"
#include "image_scale.h"
#include "jpeg_viewer.h"
#include "photo_view.h"
#include <stdio.h>

static const char *gPhotoList[] = {
    "0:/logo1.jpg",
    "0:/logo2.jpg",
    "0:/logo3.jpg",
    "0:/logo4.jpg",
    "0:/logo5.jpg",
    "0:/logo7.jpg",
    /* "0:/photo3.jpg", */
};

static const uint32_t gPhotoCount = sizeof(gPhotoList) / sizeof(gPhotoList[0]);
uint32_t gCurrentPhotoIndex = 0;

void Album_Init(void) {
  gCurrentPhotoIndex = 0;
  Album_ShowByIndex(gCurrentPhotoIndex);
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
  return 0;
}

void Album_ShowNext(void) {
  uint32_t nextIndex = gCurrentPhotoIndex + 1U;

  if (nextIndex >= gPhotoCount) {
    nextIndex = 0;
  }

  Album_ShowByIndex(nextIndex);
}

void Album_ShowPrev(void) {
  uint32_t prevIndex;

  if (gCurrentPhotoIndex == 0U) {
    prevIndex = gPhotoCount - 1U;
  } else {
    prevIndex = gCurrentPhotoIndex - 1U;
  }

  Album_ShowByIndex(prevIndex);
}

static int Album_GenerateThumb(uint32_t index, uint16_t *thumbBuf) {
  uint16_t srcW, srcH;
  uint16_t jpgW, jpgH;
  uint8_t scale = 0;
  int ret;
  uint16_t *jpgBuf = JPG_DECODE_BUF;

  if (index >= gPhotoCount) {
    return -1;
  }

  ret = JpegGetInfo(gPhotoList[index], &srcW, &srcH);
  if (ret != 0) {
    return -2;
  }

  /* Choose decode scale for thumbnail generation */
  scale = 0;
  while ((((uint32_t)srcW >> scale) > JPG_DECODE_MAX_W ||
          ((uint32_t)srcH >> scale) > JPG_DECODE_MAX_H) &&
         scale < 3U) {
    scale++;
  }

  ret =
      JpegDecodeToBufferScaled(gPhotoList[index], jpgBuf, &jpgW, &jpgH, scale);
  if (ret != 0) {
    return -3;
  }

  ScaleRGB565Contain(jpgBuf, jpgW, jpgH, thumbBuf, THUMB_W, THUMB_H, 0x0000);
  return 0;
}

uint32_t Album_GetCount(void) { return gPhotoCount; }

int Album_LoadThumbPage(uint32_t pageStartIndex) {
  int ret;

  ret = Album_GenerateThumb(pageStartIndex + 0U, THUMB_BUF0);
  if (ret != 0 && (pageStartIndex + 0U) < gPhotoCount)
    return -1;

  ret = Album_GenerateThumb(pageStartIndex + 1U, THUMB_BUF1);
  if (ret != 0 && (pageStartIndex + 1U) < gPhotoCount)
    return -2;

  ret = Album_GenerateThumb(pageStartIndex + 2U, THUMB_BUF2);
  if (ret != 0 && (pageStartIndex + 2U) < gPhotoCount)
    return -3;

  ret = Album_GenerateThumb(pageStartIndex + 3U, THUMB_BUF3);
  if (ret != 0 && (pageStartIndex + 3U) < gPhotoCount)
    return -4;

  return 0;
}

int Album_OpenFromThumb(uint32_t index) { return Album_ShowByIndex(index); }