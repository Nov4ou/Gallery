#ifndef ES8388_H
#define ES8388_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Change this include if your chip is not STM32F4.
 *
 * Examples:
 * STM32F1: #include "stm32f1xx_hal.h"
 * STM32F4: #include "stm32f4xx_hal.h"
 * STM32H7: #include "stm32h7xx_hal.h"
 */

#define ES8388_I2C_ADDR_LOW_7BIT 0x10
#define ES8388_I2C_ADDR_HIGH_7BIT 0x11

#define ES8388_REG_CHIP_CONTROL1 0x00
#define ES8388_REG_CHIP_CONTROL2 0x01
#define ES8388_REG_CHIP_POWER 0x02
#define ES8388_REG_ADC_POWER 0x03
#define ES8388_REG_DAC_POWER 0x04
#define ES8388_REG_MASTER_MODE 0x08

typedef struct {
  I2C_HandleTypeDef *hi2c;
  uint16_t devAddr;
  uint32_t timeout;

  uint32_t lastError;
  uint8_t lastReg;
  uint8_t lastValue;
} ES8388_HandleTypeDef;

typedef struct {
  uint8_t address7bit;
  uint8_t reg00;
  uint8_t reg01;
  uint8_t reg02;
  uint8_t reg03;
  uint8_t reg04;
  uint8_t reg08;
} ES8388_ProbeResultTypeDef;

void ES8388_Attach(ES8388_HandleTypeDef *dev, I2C_HandleTypeDef *hi2c,
                   uint8_t address7bit);

HAL_StatusTypeDef ES8388_IsReady(ES8388_HandleTypeDef *dev);

HAL_StatusTypeDef ES8388_ReadReg(ES8388_HandleTypeDef *dev, uint8_t reg,
                                 uint8_t *value);

HAL_StatusTypeDef ES8388_WriteReg(ES8388_HandleTypeDef *dev, uint8_t reg,
                                  uint8_t value);

uint8_t ES8388_GetLastReg(ES8388_HandleTypeDef *dev);

uint8_t ES8388_GetLastValue(ES8388_HandleTypeDef *dev);

uint32_t ES8388_GetLastError(ES8388_HandleTypeDef *dev);

HAL_StatusTypeDef ES8388_InitForPlaybackHeadphone(ES8388_HandleTypeDef *dev);

HAL_StatusTypeDef ES8388_SetHeadphoneVolume(ES8388_HandleTypeDef *dev,
                                            uint8_t volumeRegValue);

HAL_StatusTypeDef ES8388_SetSpeakerVolume(ES8388_HandleTypeDef *dev,
                                          uint8_t volumeRegValue);

HAL_StatusTypeDef ES8388_AddaConfig(ES8388_HandleTypeDef *dev, uint8_t dacen,
                                    uint8_t adcen);

HAL_StatusTypeDef ES8388_OutputConfig(ES8388_HandleTypeDef *dev, uint8_t o1en,
                                      uint8_t o2en);

HAL_StatusTypeDef ES8388_Detect(ES8388_HandleTypeDef *dev,
                                I2C_HandleTypeDef *hi2c,
                                uint8_t *foundAddress7bit);

HAL_StatusTypeDef ES8388_ReadProbeRegisters(ES8388_HandleTypeDef *dev,
                                            ES8388_ProbeResultTypeDef *result);

bool ES8388_CheckDefaultRegisters(const ES8388_ProbeResultTypeDef *result);

void ES8388_DebugI2cReadReg00(I2C_HandleTypeDef *hi2c);

void ES8388_DebugDetect(I2C_HandleTypeDef *hi2c);

#endif
