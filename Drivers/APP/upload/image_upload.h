#ifndef __IMAGE_UPLOAD_H
#define __IMAGE_UPLOAD_H

#include "ff.h"
#include <stdint.h>

typedef enum {
  UploadIdle = 0,
  UploadReceivingHeader,
  UploadReceivingData,
  UploadDone,
  UploadError
} UploadState_t;

typedef struct {
  UploadState_t state;

  uint32_t expectedSize;
  uint32_t receivedSize;

  char fileName[64];
  FIL file;

  uint8_t headerBuf[128];
  uint16_t headerLen;
} ImageUploadContext_t;

void ImageUpload_Init(ImageUploadContext_t *ctx);
int ImageUpload_ParseHeader(ImageUploadContext_t *ctx, const char *line);
int ImageUpload_StartFile(ImageUploadContext_t *ctx);
int ImageUpload_WriteData(ImageUploadContext_t *ctx, const uint8_t *data,
                          uint16_t len);
int ImageUpload_IsComplete(ImageUploadContext_t *ctx);
void ImageUpload_Close(ImageUploadContext_t *ctx);

#endif