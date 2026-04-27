#ifndef __IMAGE_UPLOAD_H
#define __IMAGE_UPLOAD_H

#include "ff.h"
#include <stdint.h>

#define IMAGE_UPLOAD_FILE_NAME_MAX 128U

typedef enum
{
    UploadIdle = 0,
    UploadReceivingHexData,
    UploadDone,
    UploadError
} UploadState_t;

typedef struct
{
    UploadState_t state;

    uint32_t expectedBinarySize;   /* Final JPG File Size (Bytes) */
    uint32_t writtenBinarySize;    /* The number of bytes written to the file. */

    char fileName[IMAGE_UPLOAD_FILE_NAME_MAX];
    FIL file;

    uint8_t halfByteValid;
    char highNibbleChar;
} ImageUploadContext_t;

void ImageUpload_Init(ImageUploadContext_t *ctx);
int ImageUpload_ParseHexHeader(ImageUploadContext_t *ctx, const char *line);
int ImageUpload_StartHexFile(ImageUploadContext_t *ctx);
int ImageUpload_WriteHexStream(ImageUploadContext_t *ctx, const uint8_t *data, uint16_t len);
int ImageUpload_IsComplete(ImageUploadContext_t *ctx);
void ImageUpload_Close(ImageUploadContext_t *ctx);

#endif
