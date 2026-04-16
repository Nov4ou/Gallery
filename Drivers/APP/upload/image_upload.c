#include "image_upload.h"
#include <stdio.h>
#include <string.h>

void ImageUpload_Init(ImageUploadContext_t *ctx) {
  if (ctx == NULL)
    return;

  memset(ctx, 0, sizeof(ImageUploadContext_t));
  ctx->state = UploadIdle;
}

int ImageUpload_ParseHeader(ImageUploadContext_t *ctx, const char *line) {
  char name[64];
  unsigned long size;

  if (ctx == NULL || line == NULL)
    return -1;

  if (sscanf(line, "IMG:%lu:%63s", &size, name) != 2) {
    return -2;
  }

  ctx->expectedSize = (uint32_t)size;
  strncpy(ctx->fileName, name, sizeof(ctx->fileName) - 1);
  ctx->fileName[sizeof(ctx->fileName) - 1] = '\0';

  return 0;
}

int ImageUpload_StartFile(ImageUploadContext_t *ctx) {
  char path[96];
  FRESULT res;

  if (ctx == NULL)
    return -1;

  snprintf(path, sizeof(path), "0:/%s", ctx->fileName);

  res = f_open(&ctx->file, path, FA_CREATE_ALWAYS | FA_WRITE);
  if (res != FR_OK) {
    ctx->state = UploadError;
    return -2;
  }

  ctx->receivedSize = 0;
  ctx->state = UploadReceivingData;
  return 0;
}

int ImageUpload_WriteData(ImageUploadContext_t *ctx, const uint8_t *data,
                          uint16_t len) {
  UINT bw;
  uint32_t remain;

  if (ctx == NULL || data == NULL)
    return -1;
  if (ctx->state != UploadReceivingData)
    return -2;

  remain = ctx->expectedSize - ctx->receivedSize;
  if (len > remain) {
    len = (uint16_t)remain;
  }

  if (f_write(&ctx->file, data, len, &bw) != FR_OK || bw != len) {
    ctx->state = UploadError;
    return -3;
  }

  ctx->receivedSize += bw;

  if (ctx->receivedSize >= ctx->expectedSize) {
    ctx->state = UploadDone;
  }

  return 0;
}

int ImageUpload_IsComplete(ImageUploadContext_t *ctx) {
  if (ctx == NULL)
    return 0;
  return (ctx->state == UploadDone);
}

void ImageUpload_Close(ImageUploadContext_t *ctx) {
  if (ctx == NULL)
    return;

  f_close(&ctx->file);

  if (ctx->state != UploadError) {
    ctx->state = UploadIdle;
  }
}