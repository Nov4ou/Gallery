#include "gt911.h"
#include "soft_i2c.h"
#include <string.h>

/* GT911 pins from your CubeMX */
#define GT911_INT_PORT GPIOB
#define GT911_INT_PIN GPIO_PIN_1

#define GT911_RST_PORT GPIOC
#define GT911_RST_PIN GPIO_PIN_13

#define GT911_I2C_ADDR 0x5D

#define GT911_REG_PRODUCT_ID 0x8140
#define GT911_REG_STATUS 0x814E
#define GT911_REG_POINT1 0x8150

static uint8_t gSwapXY = 1;
static uint8_t gMirrorX = 0;
static uint8_t gMirrorY = 0;

static void GT911_INT_Output(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = GT911_INT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GT911_INT_PORT, &GPIO_InitStruct);
}

static void GT911_INT_Input(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = GT911_INT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GT911_INT_PORT, &GPIO_InitStruct);
}

static void GT911_INT_High(void) {
  HAL_GPIO_WritePin(GT911_INT_PORT, GT911_INT_PIN, GPIO_PIN_SET);
}

static void GT911_INT_Low(void) {
  HAL_GPIO_WritePin(GT911_INT_PORT, GT911_INT_PIN, GPIO_PIN_RESET);
}

static void GT911_RST_High(void) {
  HAL_GPIO_WritePin(GT911_RST_PORT, GT911_RST_PIN, GPIO_PIN_SET);
}

static void GT911_RST_Low(void) {
  HAL_GPIO_WritePin(GT911_RST_PORT, GT911_RST_PIN, GPIO_PIN_RESET);
}

static uint8_t GT911_WriteReg(uint16_t reg, const uint8_t *buf, uint8_t len) {
  uint8_t i;

  SoftI2C_Start();
  SoftI2C_WriteByte((GT911_I2C_ADDR << 1) | 0);
  if (SoftI2C_WaitAck())
    return 1;

  SoftI2C_WriteByte((uint8_t)(reg >> 8));
  if (SoftI2C_WaitAck())
    return 1;

  SoftI2C_WriteByte((uint8_t)(reg & 0xFF));
  if (SoftI2C_WaitAck())
    return 1;

  for (i = 0; i < len; i++) {
    SoftI2C_WriteByte(buf[i]);
    if (SoftI2C_WaitAck())
      return 1;
  }

  SoftI2C_Stop();
  return 0;
}

static uint8_t GT911_ReadReg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;

    SoftI2C_Start();
    SoftI2C_WriteByte((GT911_I2C_ADDR << 1) | 0);
    if (SoftI2C_WaitAck()) return 1;

    SoftI2C_WriteByte((uint8_t)(reg >> 8));
    if (SoftI2C_WaitAck()) return 1;

    SoftI2C_WriteByte((uint8_t)(reg & 0xFF));
    if (SoftI2C_WaitAck()) return 1;

    SoftI2C_Start();
    SoftI2C_WriteByte((GT911_I2C_ADDR << 1) | 1);
    if (SoftI2C_WaitAck()) return 1;

    for (i = 0; i < len; i++)
    {
        buf[i] = SoftI2C_ReadByte(i == (len - 1) ? 0 : 1);
    }

    SoftI2C_Stop();
    return 0;
}

uint8_t GT911_Init(void) {
  uint8_t id[4] = {0};

  SoftI2C_Init();

  /* Reset sequence for address 0x5D */
  GT911_INT_Output();
  GT911_RST_Low();
  GT911_INT_Low();
  HAL_Delay(10);

  GT911_RST_High();
  HAL_Delay(50);

  GT911_INT_Input();
  HAL_Delay(10);

  if (GT911_ReadReg(GT911_REG_PRODUCT_ID, id, 4) != 0) {
    return 1;
  }

  /* Expect something like "911" */
  return 0;
}

uint8_t GT911_ReadResolution(uint16_t *xMax, uint16_t *yMax) {
  uint8_t buf[4];

  if (xMax == NULL || yMax == NULL) {
    return 1;
  }

  if (GT911_ReadReg(0x8048, buf, 4) != 0) {
    return 1;
  }

  *xMax = (uint16_t)(((uint16_t)buf[1] << 8) | buf[0]);
  *yMax = (uint16_t)(((uint16_t)buf[3] << 8) | buf[2]);

  return 0;
}

uint8_t GT911_ReadOneByte(uint16_t reg, uint8_t *value) {
  return GT911_ReadReg(reg, value, 1);
}

uint8_t GT911_ReadState(GT911_State_t *state) {
  uint8_t status = 0;
  uint8_t buf[4] = {0};
  uint8_t clear = 0;
  uint16_t x, y;

  if (state == NULL) {
    return 0;
  }

  state->pressed = 0;
  state->x = 0;
  state->y = 0;

  if (GT911_ReadReg(GT911_REG_STATUS, &status, 1) != 0) {
    return 0;
  }

  /* No new data */
  if ((status & 0x80U) == 0) {
    return 0;
  }

  /* No valid point */
  if ((status & 0x0FU) == 0) {
    GT911_WriteReg(GT911_REG_STATUS, &clear, 1);
    return 0;
  }

  /* Read first touch point: X_L, X_H, Y_L, Y_H */
  if (GT911_ReadReg(GT911_REG_POINT1, buf, 4) != 0) {
    GT911_WriteReg(GT911_REG_STATUS, &clear, 1);
    return 0;
  }

  /* Follow official logic for 0x5510 vertical mode */
  x = (uint16_t)(((uint16_t)buf[1] << 8) | buf[0]);
  y = (uint16_t)(((uint16_t)buf[3] << 8) | buf[2]);

  /* Invalid protection */
  if (x >= 480U || y >= 800U) {
    GT911_WriteReg(GT911_REG_STATUS, &clear, 1);
    return 0;
  }

  state->x = x;
  state->y = y;
  state->pressed = 1;

  GT911_WriteReg(GT911_REG_STATUS, &clear, 1);

  return 1;
}