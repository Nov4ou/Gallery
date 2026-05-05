#include "ld3320.h"
#include "gpio.h"

#if defined(LD3320_RST_GPIO_Port) && defined(LD3320_RST_Pin) &&                \
    defined(LD3320_CS_GPIO_Port) && defined(LD3320_CS_Pin)

#define LD3320_SCK_GPIO_Port GPIOA
#define LD3320_SCK_Pin GPIO_PIN_5
#define LD3320_MISO_GPIO_Port GPIOA
#define LD3320_MISO_Pin GPIO_PIN_6
#define LD3320_MOSI_GPIO_Port GPIOA
#define LD3320_MOSI_Pin GPIO_PIN_7

#define LD3320_CS_LOW()                                                        \
  HAL_GPIO_WritePin(LD3320_CS_GPIO_Port, LD3320_CS_Pin, GPIO_PIN_RESET)
#define LD3320_CS_HIGH()                                                       \
  HAL_GPIO_WritePin(LD3320_CS_GPIO_Port, LD3320_CS_Pin, GPIO_PIN_SET)

#define LD3320_RST_LOW()                                                       \
  HAL_GPIO_WritePin(LD3320_RST_GPIO_Port, LD3320_RST_Pin, GPIO_PIN_RESET)
#define LD3320_RST_HIGH()                                                      \
  HAL_GPIO_WritePin(LD3320_RST_GPIO_Port, LD3320_RST_Pin, GPIO_PIN_SET)

#define LD3320_CLK_IN 24U
#define LD3320_PLL_11 ((uint8_t)((LD3320_CLK_IN / 2U) - 1U))
#define LD3320_PLL_ASR_19                                                     \
  ((uint8_t)(LD3320_CLK_IN * 32.0 / (LD3320_PLL_11 + 1U) - 0.51))
#define LD3320_PLL_ASR_1B 0x48U
#define LD3320_PLL_ASR_1D 0x1FU
#define LD3320_MIC_VOL 0x43U

static uint8_t gLd3320PinsReady = 0U;
static uint8_t gLd3320Mode = LD_MODE_IDLE;
volatile uint8_t gLd3320AsrStatus = LD_ASR_NONE;

static void LD3320_DelayShort(void) {
  for (volatile uint32_t i = 0; i < 32U; i++) {
    __NOP();
  }
}

static void LD3320_SetSck(uint8_t level) {
  HAL_GPIO_WritePin(LD3320_SCK_GPIO_Port, LD3320_SCK_Pin,
                    level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void LD3320_SetMosi(uint8_t level) {
  HAL_GPIO_WritePin(LD3320_MOSI_GPIO_Port, LD3320_MOSI_Pin,
                    level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t LD3320_GetMiso(void) {
  return (uint8_t)(HAL_GPIO_ReadPin(LD3320_MISO_GPIO_Port, LD3320_MISO_Pin) ==
                   GPIO_PIN_SET);
}

static void LD3320_InitPins(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (gLd3320PinsReady != 0U) {
    return;
  }

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  HAL_GPIO_DeInit(GPIOA, LD3320_SCK_Pin | LD3320_MISO_Pin | LD3320_MOSI_Pin);

  GPIO_InitStruct.Pin = LD3320_SCK_Pin | LD3320_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD3320_MISO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD3320_CS_Pin | LD3320_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  LD3320_CS_HIGH();
  LD3320_RST_HIGH();
  LD3320_SetSck(1U);
  LD3320_SetMosi(1U);

  gLd3320PinsReady = 1U;
}

static void LD3320_WriteByte(uint8_t value) {
  for (uint32_t i = 0; i < 8U; i++) {
    LD3320_SetMosi((uint8_t)((value & 0x80U) != 0U));
    LD3320_DelayShort();
    LD3320_SetSck(0U);
    value <<= 1;
    LD3320_DelayShort();
    LD3320_SetSck(1U);
  }
}

static uint8_t LD3320_ReadByte(void) {
  uint8_t value = 0U;

  for (uint32_t i = 0; i < 8U; i++) {
    value <<= 1;
    if (LD3320_GetMiso() != 0U) {
      value |= 0x01U;
    }
    LD3320_DelayShort();
    LD3320_SetSck(0U);
    LD3320_DelayShort();
    LD3320_SetSck(1U);
  }

  return value;
}

void LD3320_WriteReg(uint8_t reg, uint8_t value) {
  LD3320_InitPins();

  LD3320_CS_LOW();
  LD3320_DelayShort();
  LD3320_WriteByte(0x04U);
  LD3320_WriteByte(reg);
  LD3320_WriteByte(value);
  LD3320_DelayShort();
  LD3320_CS_HIGH();
}

uint8_t LD3320_ReadReg(uint8_t reg) {
  uint8_t value;

  LD3320_InitPins();

  LD3320_CS_LOW();
  LD3320_DelayShort();
  LD3320_WriteByte(0x05U);
  LD3320_WriteByte(reg);
  LD3320_DelayShort();
  value = LD3320_ReadByte();
  LD3320_DelayShort();
  LD3320_CS_HIGH();

  return value;
}

void LD3320_Reset(void) {
  LD3320_InitPins();

  LD3320_RST_HIGH();
  HAL_Delay(5);
  LD3320_RST_LOW();
  HAL_Delay(5);
  LD3320_RST_HIGH();
  HAL_Delay(5);
  LD3320_CS_LOW();
  HAL_Delay(5);
  LD3320_CS_HIGH();
  HAL_Delay(5);
}

void LD3320_InitCommon(void) {
  (void)LD3320_ReadReg(0x06U);
  LD3320_WriteReg(0x17U, 0x35U);
  HAL_Delay(5);
  (void)LD3320_ReadReg(0x06U);

  LD3320_WriteReg(0x89U, 0x03U);
  HAL_Delay(5);
  LD3320_WriteReg(0xCFU, 0x43U);
  HAL_Delay(5);
  LD3320_WriteReg(0xCBU, 0x02U);

  LD3320_WriteReg(0x11U, LD3320_PLL_11);
  LD3320_WriteReg(0x1EU, 0x00U);
  LD3320_WriteReg(0x19U, LD3320_PLL_ASR_19);
  LD3320_WriteReg(0x1BU, LD3320_PLL_ASR_1B);
  LD3320_WriteReg(0x1DU, LD3320_PLL_ASR_1D);
  HAL_Delay(5);

  LD3320_WriteReg(0xCDU, 0x04U);
  LD3320_WriteReg(0x17U, 0x4CU);
  HAL_Delay(5);
  LD3320_WriteReg(0xB9U, 0x00U);
  LD3320_WriteReg(0xCFU, 0x4FU);
  LD3320_WriteReg(0x6FU, 0xFFU);
}

void LD3320_InitAsr(void) {
  gLd3320Mode = LD_MODE_ASR_RUN;
  LD3320_InitCommon();

  LD3320_WriteReg(0xBDU, 0x00U);
  LD3320_WriteReg(0x17U, 0x48U);
  HAL_Delay(5);

  LD3320_WriteReg(0x3CU, 0x80U);
  LD3320_WriteReg(0x3EU, 0x07U);
  LD3320_WriteReg(0x38U, 0xFFU);
  LD3320_WriteReg(0x3AU, 0x07U);
  LD3320_WriteReg(0x40U, 0x00U);
  LD3320_WriteReg(0x42U, 0x08U);
  LD3320_WriteReg(0x44U, 0x00U);
  LD3320_WriteReg(0x46U, 0x08U);
  HAL_Delay(5);
}

void LD3320_AsrStart(void) { LD3320_InitAsr(); }

void LD3320_ProcessInt(void) {
  uint8_t status;
  uint8_t count = 0U;

  status = LD3320_ReadReg(0x2BU);
  LD3320_WriteReg(0x29U, 0x00U);
  LD3320_WriteReg(0x02U, 0x00U);

  if ((status & 0x10U) != 0U && LD3320_ReadReg(0xB2U) == 0x21U &&
      LD3320_ReadReg(0xBFU) == 0x35U) {
    count = LD3320_ReadReg(0xBAU);
    if (count > 0U && count <= 14U) {
      gLd3320AsrStatus = LD_ASR_FOUNDOK;
    } else {
      gLd3320AsrStatus = LD_ASR_FOUNDZERO;
    }
  } else {
    gLd3320AsrStatus = LD_ASR_FOUNDZERO;
  }

  LD3320_WriteReg(0x2BU, 0x00U);
  LD3320_WriteReg(0x1CU, 0x00U);
  LD3320_WriteReg(0x29U, 0x00U);
  LD3320_WriteReg(0x02U, 0x00U);
  LD3320_WriteReg(0x2BU, 0x00U);
  LD3320_WriteReg(0xBAU, 0x00U);
  LD3320_WriteReg(0xBCU, 0x00U);
  LD3320_WriteReg(0x08U, 0x01U);
  LD3320_WriteReg(0x08U, 0x00U);
}

uint8_t LD3320_CheckAsrBusyFlagB2(void) {
  for (uint32_t i = 0; i < 5U; i++) {
    if (LD3320_ReadReg(0xB2U) == 0x21U) {
      return 1U;
    }
    HAL_Delay(20);
  }
  return 0U;
}

uint8_t LD3320_AsrRun(void) {
  LD3320_WriteReg(0x35U, LD3320_MIC_VOL);
  LD3320_WriteReg(0x1CU, 0x09U);
  LD3320_WriteReg(0xBDU, 0x20U);
  LD3320_WriteReg(0x08U, 0x01U);
  HAL_Delay(5);
  LD3320_WriteReg(0x08U, 0x00U);
  HAL_Delay(5);

  if (LD3320_CheckAsrBusyFlagB2() == 0U) {
    return 0U;
  }

  LD3320_WriteReg(0xB2U, 0xFFU);
  LD3320_WriteReg(0x37U, 0x06U);
  HAL_Delay(5);
  LD3320_WriteReg(0x37U, 0x06U);
  HAL_Delay(5);
  LD3320_WriteReg(0x1CU, 0x0BU);
  LD3320_WriteReg(0x29U, 0x10U);
  LD3320_WriteReg(0xBDU, 0x00U);
  return 1U;
}

uint8_t LD3320_AsrAddFixed(void) {
  static const char *const kRecog[] = {"ni hao", "bei jing", "shang hai"};
  static const uint8_t kCode[] = {LD3320_CODE_CMD, LD3320_CODE_1KL1,
                                  LD3320_CODE_1KL2};

  for (uint32_t k = 0; k < (sizeof(kCode) / sizeof(kCode[0])); k++) {
    uint32_t len = 0U;

    if (LD3320_CheckAsrBusyFlagB2() == 0U) {
      return 0U;
    }

    LD3320_WriteReg(0xC1U, kCode[k]);
    LD3320_WriteReg(0xC3U, 0x00U);
    LD3320_WriteReg(0x08U, 0x04U);
    HAL_Delay(1);
    LD3320_WriteReg(0x08U, 0x00U);
    HAL_Delay(1);

    while (kRecog[k][len] != '\0') {
      LD3320_WriteReg(0x05U, (uint8_t)kRecog[k][len]);
      len++;
    }

    LD3320_WriteReg(0xB9U, (uint8_t)len);
    LD3320_WriteReg(0xB2U, 0xFFU);
    LD3320_WriteReg(0x37U, 0x04U);
  }

  return 1U;
}

uint8_t LD3320_RunAsr(void) {
  for (uint32_t i = 0; i < 5U; i++) {
    LD3320_AsrStart();
    HAL_Delay(5);
    if (LD3320_AsrAddFixed() == 0U) {
      LD3320_Reset();
      HAL_Delay(5);
      continue;
    }
    HAL_Delay(5);
    if (LD3320_AsrRun() == 0U) {
      LD3320_Reset();
      HAL_Delay(5);
      continue;
    }
    return 1U;
  }

  return 0U;
}

uint8_t LD3320_GetResult(void) { return LD3320_ReadReg(0xC5U); }

#else

void LD3320_Reset(void) {}
void LD3320_WriteReg(uint8_t reg, uint8_t value) {
  (void)reg;
  (void)value;
}
uint8_t LD3320_ReadReg(uint8_t reg) {
  (void)reg;
  return 0U;
}
void LD3320_InitCommon(void) {}
void LD3320_InitAsr(void) {}
void LD3320_AsrStart(void) {}
void LD3320_ProcessInt(void) {}
uint8_t LD3320_CheckAsrBusyFlagB2(void) { return 0U; }
uint8_t LD3320_AsrRun(void) { return 0U; }
uint8_t LD3320_AsrAddFixed(void) { return 0U; }
uint8_t LD3320_RunAsr(void) { return 0U; }
uint8_t LD3320_GetResult(void) { return 0U; }

#endif
