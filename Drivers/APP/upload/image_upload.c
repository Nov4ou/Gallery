#include "image_upload.h"
#include <stdio.h>
#include <string.h>

static void TrimLineEnd(char *s) {
  int len;

  if (s == NULL) {
    return;
  }

  len = (int)strlen(s);

  while (len > 0 && (s[len - 1] == '\r' || s[len - 1] == '\n' ||
                     s[len - 1] == ' ' || s[len - 1] == '\t')) {
    s[len - 1] = '\0';
    len--;
  }
}

static int HexCharToVal(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }

  return -1;
}

void ImageUpload_Init(ImageUploadContext_t *ctx) {
  if (ctx == NULL) {
    return;
  }

  memset(ctx, 0, sizeof(ImageUploadContext_t));
  ctx->state = UploadIdle;
}

int ImageUpload_ParseHexHeader(ImageUploadContext_t *ctx, const char *line) {
  char name[64];
  unsigned long size;

  if (ctx == NULL || line == NULL) {
    printf("ParseHexHeader: invalid param\r\n");
    return -1;
  }

  printf("ParseHexHeader input = [%s]\r\n", line);

  if (sscanf(line, "IMGHEX:%lu:%63s", &size, name) != 2) {
    printf("ParseHexHeader: sscanf failed\r\n");
    return -2;
  }

  TrimLineEnd(name);

  ctx->expectedBinarySize = (uint32_t)size;
  strncpy(ctx->fileName, name, sizeof(ctx->fileName) - 1);
  ctx->fileName[sizeof(ctx->fileName) - 1] = '\0';

  printf("ParseHexHeader ok: expectedBinarySize=%lu fileName=%s\r\n",
         (unsigned long)ctx->expectedBinarySize, ctx->fileName);

  return 0;
}

int ImageUpload_StartHexFile(ImageUploadContext_t *ctx) {
  char path[96];
  FRESULT res;

  if (ctx == NULL) {
    printf("StartHexFile: ctx NULL\r\n");
    return -1;
  }

  snprintf(path, sizeof(path), "0:/%s", ctx->fileName);

  printf("StartHexFile path=[%s]\r\n", path);
  printf("expectedBinarySize=%lu\r\n", (unsigned long)ctx->expectedBinarySize);

  res = f_open(&ctx->file, path, FA_CREATE_ALWAYS | FA_WRITE);
  printf("f_open res=%d\r\n", res);

  if (res != FR_OK) {
    ctx->state = UploadError;
    return -2;
  }

  ctx->writtenBinarySize = 0;
  ctx->halfByteValid = 0;
  ctx->highNibbleChar = 0;
  ctx->state = UploadReceivingHexData;

  printf("StartHexFile OK\r\n");
  return 0;
}

int ImageUpload_WriteHexStream(ImageUploadContext_t *ctx, const uint8_t *data,
                               uint16_t len) {
  uint16_t i;
  UINT bw;
  uint8_t outByte;
  int hi, lo;
  uint32_t debugCount = 0;

  if (ctx == NULL || data == NULL) {
    printf("WriteHexStream: invalid param\r\n");
    return -1;
  }

  if (ctx->state != UploadReceivingHexData) {
    printf("WriteHexStream: wrong state=%d\r\n", ctx->state);
    return -2;
  }

  printf("WriteHexStream: input len=%u\r\n", len);

  for (i = 0; i < len; i++) {
    char c = (char)data[i];

    if (c == '\r' || c == '\n' || c == ' ' || c == '\t') {
      continue;
    }

    if (HexCharToVal(c) < 0) {
      printf("WriteHexStream: invalid hex char '%c' (0x%02X)\r\n", c,
             (unsigned char)c);
      ctx->state = UploadError;
      return -3;
    }

    if (!ctx->halfByteValid) {
      ctx->highNibbleChar = c;
      ctx->halfByteValid = 1;
    } else {
      hi = HexCharToVal(ctx->highNibbleChar);
      lo = HexCharToVal(c);

      if (hi < 0 || lo < 0) {
        printf("WriteHexStream: hex convert failed hi=%d lo=%d\r\n", hi, lo);
        ctx->state = UploadError;
        return -4;
      }

      outByte = (uint8_t)((hi << 4) | lo);

      if (debugCount < 16) {
        printf("byte[%lu] = 0x%02X\r\n", (unsigned long)ctx->writtenBinarySize,
               outByte);
        debugCount++;
      }

      if (f_write(&ctx->file, &outByte, 1, &bw) != FR_OK || bw != 1) {
        printf("WriteHexStream: f_write failed\r\n");
        ctx->state = UploadError;
        return -5;
      }

      ctx->writtenBinarySize++;
      ctx->halfByteValid = 0;

      if ((ctx->writtenBinarySize % 128U) == 0U) {
        printf("writtenBinarySize=%lu/%lu\r\n",
               (unsigned long)ctx->writtenBinarySize,
               (unsigned long)ctx->expectedBinarySize);
      }

      if (ctx->writtenBinarySize >= ctx->expectedBinarySize) {
        ctx->state = UploadDone;
        printf("WriteHexStream: upload done, total=%lu\r\n",
               (unsigned long)ctx->writtenBinarySize);
        return 0;
      }
    }
  }

  return 0;
}

int ImageUpload_IsComplete(ImageUploadContext_t *ctx) {
  if (ctx == NULL) {
    return 0;
  }

  return (ctx->state == UploadDone);
}

void ImageUpload_Close(ImageUploadContext_t *ctx) {
  if (ctx == NULL) {
    return;
  }

  f_close(&ctx->file);

  if (ctx->state != UploadError) {
    ctx->state = UploadIdle;
  }
}