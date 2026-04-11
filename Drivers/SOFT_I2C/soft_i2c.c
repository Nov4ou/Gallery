#include "soft_i2c.h"

/* GT911 pins from your CubeMX */
#define GT911_SCL_PORT GPIOB
#define GT911_SCL_PIN GPIO_PIN_0

#define GT911_SDA_PORT GPIOF
#define GT911_SDA_PIN GPIO_PIN_11

static void I2C_Delay(void) {
  volatile uint32_t i;
  for (i = 0; i < 150; i++) {
    __NOP();
  }
}

static void SDA_Output(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = GT911_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GT911_SDA_PORT, &GPIO_InitStruct);
}

static void SDA_Input(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = GT911_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GT911_SDA_PORT, &GPIO_InitStruct);
}

static void SCL_High(void) {
  HAL_GPIO_WritePin(GT911_SCL_PORT, GT911_SCL_PIN, GPIO_PIN_SET);
}

static void SCL_Low(void) {
  HAL_GPIO_WritePin(GT911_SCL_PORT, GT911_SCL_PIN, GPIO_PIN_RESET);
}

static void SDA_High(void) {
  HAL_GPIO_WritePin(GT911_SDA_PORT, GT911_SDA_PIN, GPIO_PIN_SET);
}

static void SDA_Low(void) {
  HAL_GPIO_WritePin(GT911_SDA_PORT, GT911_SDA_PIN, GPIO_PIN_RESET);
}

static uint8_t SDA_Read(void) {
  return (HAL_GPIO_ReadPin(GT911_SDA_PORT, GT911_SDA_PIN) == GPIO_PIN_SET) ? 1U
                                                                           : 0U;
}

void SoftI2C_Init(void) {
  SDA_Output();
  SDA_High();
  SCL_High();
}

void SoftI2C_Start(void) {
  SDA_Output();
  SDA_High();
  SCL_High();
  I2C_Delay();

  SDA_Low();
  I2C_Delay();

  SCL_Low();
  I2C_Delay();
}

void SoftI2C_Stop(void) {
  SDA_Output();

  SCL_Low();
  SDA_Low();
  I2C_Delay();

  SCL_High();
  I2C_Delay();

  SDA_High();
  I2C_Delay();
}

uint8_t SoftI2C_WaitAck(void) {
  uint16_t timeout = 0;

  SDA_Input();
  I2C_Delay();

  SCL_High();
  I2C_Delay();

  while (SDA_Read()) {
    timeout++;
    if (timeout > 500) {
      SCL_Low();
      SoftI2C_Stop();
      return 1;
    }
  }

  SCL_Low();
  I2C_Delay();
  return 0;
}

void SoftI2C_SendAck(uint8_t ack) {
  SDA_Output();
  SCL_Low();

  if (ack) {
    SDA_Low(); /* ACK */
  } else {
    SDA_High(); /* NACK */
  }

  I2C_Delay();
  SCL_High();
  I2C_Delay();
  SCL_Low();
}

void SoftI2C_WriteByte(uint8_t data) {
  uint8_t i;

  SDA_Output();

  for (i = 0; i < 8; i++) {
    SCL_Low();
    I2C_Delay();

    if (data & 0x80) {
      SDA_High();
    } else {
      SDA_Low();
    }

    data <<= 1;
    I2C_Delay();

    SCL_High();
    I2C_Delay();
  }

  SCL_Low();
  I2C_Delay();
}

uint8_t SoftI2C_ReadByte(uint8_t ack) {
  uint8_t i;
  uint8_t data = 0;

  SDA_Input();

  for (i = 0; i < 8; i++) {
    data <<= 1;

    SCL_Low();
    I2C_Delay();

    SCL_High();
    I2C_Delay();

    if (SDA_Read()) {
      data |= 0x01;
    }
  }

  SCL_Low();
  I2C_Delay();

  SoftI2C_SendAck(ack);
  return data;
}