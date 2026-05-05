#ifndef LD3320_H
#define LD3320_H

#include "main.h"
#include <stdint.h>

#define LD_MODE_IDLE 0x00U
#define LD_MODE_ASR_RUN 0x08U
#define LD_MODE_MP3 0x40U

#define LD_ASR_NONE 0x00U
#define LD_ASR_RUNING 0x01U
#define LD_ASR_FOUNDOK 0x10U
#define LD_ASR_FOUNDZERO 0x11U
#define LD_ASR_ERROR 0x31U

#define LD3320_CODE_CMD 0x00U
#define LD3320_CODE_DMCS 0x01U
#define LD3320_CODE_CSWB 0x02U
#define LD3320_CODE_1KL1 0x03U
#define LD3320_CODE_1KL2 0x04U
#define LD3320_CODE_1KL3 0x05U
#define LD3320_CODE_1KL4 0x06U
#define LD3320_CODE_2KL1 0x18U
#define LD3320_CODE_2KL2 0x19U
#define LD3320_CODE_2KL3 0x1AU
#define LD3320_CODE_2KL4 0x1BU
#define LD3320_CODE_3KL1 0x1CU
#define LD3320_CODE_3KL2 0x1DU
#define LD3320_CODE_3KL3 0x1EU
#define LD3320_CODE_3KL4 0x1FU
#define LD3320_CODE_4KL1 0x20U
#define LD3320_CODE_4KL2 0x21U
#define LD3320_CODE_4KL3 0x22U
#define LD3320_CODE_4KL4 0x23U
#define LD3320_CODE_5KL1 0x24U

extern volatile uint8_t gLd3320AsrStatus;

void LD3320_Reset(void);
void LD3320_WriteReg(uint8_t reg, uint8_t value);
uint8_t LD3320_ReadReg(uint8_t reg);
void LD3320_InitCommon(void);
void LD3320_InitAsr(void);
void LD3320_AsrStart(void);
uint8_t LD3320_CheckAsrBusyFlagB2(void);
uint8_t LD3320_AsrRun(void);
uint8_t LD3320_AsrAddFixed(void);
uint8_t LD3320_RunAsr(void);
void LD3320_ProcessInt(void);
uint8_t LD3320_GetResult(void);

#endif
