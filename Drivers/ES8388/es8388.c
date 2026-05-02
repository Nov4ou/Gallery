#include "es8388.h"

typedef struct {
  uint8_t reg;
  uint8_t value;
} ES8388_RegValueTypeDef;

/* ES8388 is connected on PB8/PB9 on this board. Use software I2C here to
 * match the official example's access method and avoid the hardware-I2C
 * stability issues we observed on this link. */
#define ES8388_SCL_PORT GPIOB
#define ES8388_SCL_PIN GPIO_PIN_8
#define ES8388_SDA_PORT GPIOB
#define ES8388_SDA_PIN GPIO_PIN_9

static void ES8388_SoftI2C_Stop(void);

static void ES8388_I2C_Delay(void) {
  for (volatile uint32_t i = 0; i < 240; i++) {
    __NOP();
  }
}

static void ES8388_SCL_Output(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = ES8388_SCL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(ES8388_SCL_PORT, &GPIO_InitStruct);
}

static void ES8388_SDA_Output(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = ES8388_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(ES8388_SDA_PORT, &GPIO_InitStruct);
}

static void ES8388_SCL_High(void) {
  HAL_GPIO_WritePin(ES8388_SCL_PORT, ES8388_SCL_PIN, GPIO_PIN_SET);
}

static void ES8388_SCL_Low(void) {
  HAL_GPIO_WritePin(ES8388_SCL_PORT, ES8388_SCL_PIN, GPIO_PIN_RESET);
}

static void ES8388_SDA_High(void) {
  HAL_GPIO_WritePin(ES8388_SDA_PORT, ES8388_SDA_PIN, GPIO_PIN_SET);
}

static void ES8388_SDA_Low(void) {
  HAL_GPIO_WritePin(ES8388_SDA_PORT, ES8388_SDA_PIN, GPIO_PIN_RESET);
}

static void ES8388_SDA_Write(uint8_t level) {
  if (level) {
    ES8388_SDA_High();
  } else {
    ES8388_SDA_Low();
  }
}

static uint8_t ES8388_SDA_Read(void) {
  return (HAL_GPIO_ReadPin(ES8388_SDA_PORT, ES8388_SDA_PIN) == GPIO_PIN_SET)
             ? 1U
             : 0U;
}

static void ES8388_SoftI2C_Init(void) {
  __HAL_RCC_GPIOB_CLK_ENABLE();
  ES8388_SCL_Output();
  ES8388_SDA_Output();
  ES8388_SoftI2C_Stop();
}

static void ES8388_SoftI2C_Start(void) {
  ES8388_SDA_High();
  ES8388_SCL_High();
  ES8388_I2C_Delay();
  ES8388_SDA_Low();
  ES8388_I2C_Delay();
  ES8388_SCL_Low();
  ES8388_I2C_Delay();
}

static void ES8388_SoftI2C_Stop(void) {
  ES8388_SDA_Low();
  ES8388_I2C_Delay();
  ES8388_SCL_High();
  ES8388_I2C_Delay();
  ES8388_SDA_High();
  ES8388_I2C_Delay();
}

static uint8_t ES8388_SoftI2C_WaitAck(void) {
  uint8_t timeout = 0;

  ES8388_SDA_High();
  ES8388_I2C_Delay();
  ES8388_SCL_High();
  ES8388_I2C_Delay();

  while (ES8388_SDA_Read()) {
    timeout++;
    if (timeout > 250U) {
      ES8388_SoftI2C_Stop();
      return 1U;
    }
  }

  ES8388_SCL_Low();
  ES8388_I2C_Delay();
  return 0U;
}

static void ES8388_SoftI2C_SendAck(uint8_t ack) {
  if (ack) {
    ES8388_SDA_Low();
  } else {
    ES8388_SDA_High();
  }
  ES8388_I2C_Delay();
  ES8388_SCL_High();
  ES8388_I2C_Delay();
  ES8388_SCL_Low();
  ES8388_I2C_Delay();
  if (ack) {
    ES8388_SDA_High();
    ES8388_I2C_Delay();
  }
}

static void ES8388_SoftI2C_WriteByte(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    ES8388_SDA_Write((uint8_t)((data & 0x80U) >> 7));
    ES8388_I2C_Delay();
    ES8388_SCL_High();
    ES8388_I2C_Delay();
    ES8388_SCL_Low();
    data <<= 1;
  }

  ES8388_SDA_High();
}

static uint8_t ES8388_SoftI2C_ReadByte(uint8_t ack) {
  uint8_t data = 0;

  for (uint8_t i = 0; i < 8; i++) {
    data <<= 1;
    ES8388_SCL_High();
    ES8388_I2C_Delay();
    if (ES8388_SDA_Read()) {
      data |= 0x01U;
    }
    ES8388_SCL_Low();
    ES8388_I2C_Delay();
  }

  ES8388_SoftI2C_SendAck(ack ? 1U : 0U);
  return data;
}

void ES8388_Attach(ES8388_HandleTypeDef *dev, I2C_HandleTypeDef *hi2c,
                   uint8_t address7bit) {
  dev->hi2c = hi2c;
  dev->devAddr = (uint16_t)(address7bit << 1);
  dev->timeout = 1000;

  dev->lastError = 0;
  dev->lastReg = 0xFF;
  dev->lastValue = 0xFF;

  ES8388_SoftI2C_Init();
}

HAL_StatusTypeDef ES8388_IsReady(ES8388_HandleTypeDef *dev) {
  if (dev == NULL) {
    return HAL_ERROR;
  }

  ES8388_SoftI2C_Init();
  ES8388_SoftI2C_Start();
  ES8388_SoftI2C_WriteByte((uint8_t)(dev->devAddr | 0U));
  if (ES8388_SoftI2C_WaitAck()) {
    dev->lastError = HAL_I2C_ERROR_TIMEOUT;
    return HAL_ERROR;
  }
  ES8388_SoftI2C_Stop();
  dev->lastError = 0;
  return HAL_OK;
}

HAL_StatusTypeDef ES8388_ReadReg(ES8388_HandleTypeDef *dev, uint8_t reg,
                                 uint8_t *value) {
  if (dev == NULL || dev->hi2c == NULL || value == NULL) {
    return HAL_ERROR;
  }

  for (uint32_t attempt = 0; attempt < 3; attempt++) {
    ES8388_SoftI2C_Start();
    ES8388_SoftI2C_WriteByte((uint8_t)(dev->devAddr | 0U));
    if (ES8388_SoftI2C_WaitAck() == 0U) {
      ES8388_SoftI2C_WriteByte(reg);
      if (ES8388_SoftI2C_WaitAck() == 0U) {
        ES8388_SoftI2C_Start();
        ES8388_SoftI2C_WriteByte((uint8_t)(dev->devAddr | 1U));
        if (ES8388_SoftI2C_WaitAck() == 0U) {
          *value = ES8388_SoftI2C_ReadByte(0U);
          ES8388_SoftI2C_Stop();
          dev->lastError = 0;
          return HAL_OK;
        }
      }
    }

    ES8388_SoftI2C_Stop();
    dev->lastError = HAL_I2C_ERROR_TIMEOUT;
    HAL_Delay(1);
    ES8388_SoftI2C_Init();
    if (attempt < 2U) {
      HAL_Delay(2);
    }
  }

  return HAL_ERROR;
}

HAL_StatusTypeDef ES8388_WriteReg(ES8388_HandleTypeDef *dev, uint8_t reg,
                                  uint8_t value) {
  if (dev == NULL || dev->hi2c == NULL) {
    return HAL_ERROR;
  }

  dev->lastReg = reg;
  dev->lastValue = value;
  dev->lastError = 0;

  for (uint32_t attempt = 0; attempt < 3; attempt++) {
    ES8388_SoftI2C_Start();
    ES8388_SoftI2C_WriteByte((uint8_t)(dev->devAddr | 0U));
    if (ES8388_SoftI2C_WaitAck() == 0U) {
      ES8388_SoftI2C_WriteByte(reg);
      if (ES8388_SoftI2C_WaitAck() == 0U) {
        ES8388_SoftI2C_WriteByte(value);
        if (ES8388_SoftI2C_WaitAck() == 0U) {
          ES8388_SoftI2C_Stop();
          dev->lastError = 0;
          return HAL_OK;
        }
      }
    }

    ES8388_SoftI2C_Stop();
    dev->lastError = HAL_I2C_ERROR_TIMEOUT;
    HAL_Delay(1);
    ES8388_SoftI2C_Init();
    if (attempt < 2U) {
      HAL_Delay(2);
    }
  }

  return HAL_ERROR;
}

uint8_t ES8388_GetLastReg(ES8388_HandleTypeDef *dev) {
  if (dev == NULL) {
    return 0xFF;
  }

  return dev->lastReg;
}

uint8_t ES8388_GetLastValue(ES8388_HandleTypeDef *dev) {
  if (dev == NULL) {
    return 0xFF;
  }

  return dev->lastValue;
}

uint32_t ES8388_GetLastError(ES8388_HandleTypeDef *dev) {
  if (dev == NULL) {
    return 0xFFFFFFFF;
  }

  return dev->lastError;
}

static HAL_StatusTypeDef
ES8388_WriteRegisterList(ES8388_HandleTypeDef *dev,
                         const ES8388_RegValueTypeDef *regs, uint32_t count) {
  for (uint32_t i = 0; i < count; i++) {
    if (ES8388_WriteReg(dev, regs[i].reg, regs[i].value) != HAL_OK) {
      printf("ES8388 init write failed: reg=0x%02X value=0x%02X err=0x%08lX\r\n",
             regs[i].reg, regs[i].value, (unsigned long)dev->lastError);
      return HAL_ERROR;
    }
    HAL_Delay(1);
  }

  return HAL_OK;
}

HAL_StatusTypeDef ES8388_InitForPlaybackHeadphone(ES8388_HandleTypeDef *dev) {
  static const ES8388_RegValueTypeDef kInitSeq[] = {
      {0x00, 0x80}, {0x00, 0x00},
      {0x01, 0x58}, {0x01, 0x50},
      {0x02, 0xF3}, {0x02, 0xF0},
      {0x03, 0x09},
      {0x00, 0x06},
      {0x04, 0x00},
      {0x08, 0x00},
      {0x2B, 0x80},
      {0x09, 0x88},
      {0x0C, 0x4C},
      {0x0D, 0x02},
      {0x10, 0x00}, {0x11, 0x00},
      {0x17, 0x18}, {0x18, 0x02},
      {0x1A, 0x00}, {0x1B, 0x00},
      {0x27, 0xB8}, {0x2A, 0xB8},
  };

  if (dev == NULL) {
    return HAL_ERROR;
  }

  if (ES8388_WriteRegisterList(dev, kInitSeq,
                               sizeof(kInitSeq) / sizeof(kInitSeq[0])) !=
      HAL_OK) {
    return HAL_ERROR;
  }

  HAL_Delay(100);
  return HAL_OK;
}

HAL_StatusTypeDef ES8388_SetHeadphoneVolume(ES8388_HandleTypeDef *dev,
                                            uint8_t volumeRegValue) {
  if (dev == NULL) {
    return HAL_ERROR;
  }

  if (ES8388_WriteReg(dev, 0x2E, volumeRegValue) != HAL_OK) {
    return HAL_ERROR;
  }

  return ES8388_WriteReg(dev, 0x2F, volumeRegValue);
}

HAL_StatusTypeDef ES8388_SetSpeakerVolume(ES8388_HandleTypeDef *dev,
                                          uint8_t volumeRegValue) {
  if (dev == NULL) {
    return HAL_ERROR;
  }

  if (ES8388_WriteReg(dev, 0x30, volumeRegValue) != HAL_OK) {
    return HAL_ERROR;
  }

  return ES8388_WriteReg(dev, 0x31, volumeRegValue);
}

HAL_StatusTypeDef ES8388_AddaConfig(ES8388_HandleTypeDef *dev, uint8_t dacen,
                                    uint8_t adcen) {
  uint8_t tempreg = 0;

  if (dev == NULL) {
    return HAL_ERROR;
  }

  tempreg |= (uint8_t)((!dacen) << 0);
  tempreg |= (uint8_t)((!adcen) << 1);
  tempreg |= (uint8_t)((!dacen) << 2);
  tempreg |= (uint8_t)((!adcen) << 3);
  return ES8388_WriteReg(dev, 0x02, tempreg);
}

HAL_StatusTypeDef ES8388_OutputConfig(ES8388_HandleTypeDef *dev, uint8_t o1en,
                                      uint8_t o2en) {
  uint8_t tempreg = 0;

  if (dev == NULL) {
    return HAL_ERROR;
  }

  tempreg |= (uint8_t)(o1en ? (3U << 4) : 0U);
  tempreg |= (uint8_t)(o2en ? (3U << 2) : 0U);
  return ES8388_WriteReg(dev, 0x04, tempreg);
}

HAL_StatusTypeDef ES8388_Detect(ES8388_HandleTypeDef *dev,
                                I2C_HandleTypeDef *hi2c,
                                uint8_t *foundAddress7bit)
{
    if (dev == NULL || hi2c == NULL || foundAddress7bit == NULL)
    {
        return HAL_ERROR;
    }

    ES8388_Attach(dev, hi2c, ES8388_I2C_ADDR_LOW_7BIT);

    if (ES8388_IsReady(dev) == HAL_OK)
    {
        *foundAddress7bit = ES8388_I2C_ADDR_LOW_7BIT;
        return HAL_OK;
    }

    ES8388_Attach(dev, hi2c, ES8388_I2C_ADDR_HIGH_7BIT);

    if (ES8388_IsReady(dev) == HAL_OK)
    {
        *foundAddress7bit = ES8388_I2C_ADDR_HIGH_7BIT;
        return HAL_OK;
    }

    *foundAddress7bit = 0x00;
    dev->devAddr = 0x0000;

    return HAL_ERROR;
}

HAL_StatusTypeDef ES8388_ReadProbeRegisters(ES8388_HandleTypeDef *dev,
                                            ES8388_ProbeResultTypeDef *result) {
  HAL_StatusTypeDef status;

  if (dev == NULL || result == NULL) {
    return HAL_ERROR;
  }

  result->address7bit = (uint8_t)(dev->devAddr >> 1);

  HAL_Delay(2);

  status = ES8388_ReadReg(dev, 0x00, &result->reg00);
  printf("Read REG00: status=%d, error=0x%08lX, value=0x%02X\r\n", status,
         HAL_I2C_GetError(dev->hi2c), result->reg00);
  if (status != HAL_OK)
    return status;
  HAL_Delay(1);

  status = ES8388_ReadReg(dev, 0x01, &result->reg01);
  printf("Read REG01: status=%d, error=0x%08lX, value=0x%02X\r\n", status,
         HAL_I2C_GetError(dev->hi2c), result->reg01);
  if (status != HAL_OK)
    return status;
  HAL_Delay(1);

  status = ES8388_ReadReg(dev, 0x02, &result->reg02);
  printf("Read REG02: status=%d, error=0x%08lX, value=0x%02X\r\n", status,
         HAL_I2C_GetError(dev->hi2c), result->reg02);
  if (status != HAL_OK)
    return status;
  HAL_Delay(1);

  status = ES8388_ReadReg(dev, 0x03, &result->reg03);
  printf("Read REG03: status=%d, error=0x%08lX, value=0x%02X\r\n", status,
         HAL_I2C_GetError(dev->hi2c), result->reg03);
  if (status != HAL_OK)
    return status;
  HAL_Delay(1);

  status = ES8388_ReadReg(dev, 0x04, &result->reg04);
  printf("Read REG04: status=%d, error=0x%08lX, value=0x%02X\r\n", status,
         HAL_I2C_GetError(dev->hi2c), result->reg04);
  if (status != HAL_OK)
    return status;
  HAL_Delay(1);

  status = ES8388_ReadReg(dev, 0x08, &result->reg08);
  printf("Read REG08: status=%d, error=0x%08lX, value=0x%02X\r\n", status,
         HAL_I2C_GetError(dev->hi2c), result->reg08);
  if (status != HAL_OK)
    return status;

  return HAL_OK;
}

bool ES8388_CheckDefaultRegisters(const ES8388_ProbeResultTypeDef *result) {
  /*
   * This check is only valid before you configure the codec.
   * If your program has already written ES8388 registers, these values may
   * change.
   *
   * REG03 (ADC power related) has been observed as 0x3F / 0x7F on this board
   * even though some reference material lists 0xFC. Its power-on value is not
   * stable enough here to use as a hard gate, so this check focuses on the
   * more consistent registers instead.
   */
  if (result->reg00 != 0x06)
    return false;
  if (result->reg01 != 0x5C)
    return false;
  if (result->reg02 != 0xC3)
    return false;
  if (result->reg04 != 0xC0)
    return false;
  if (result->reg08 != 0x80)
    return false;

  return true;
}

void ES8388_DebugI2cReadReg00(I2C_HandleTypeDef *hi2c) {
  uint16_t devAddr = 0x10 << 1;
  uint8_t reg = 0x00;
  uint8_t value = 0;
  HAL_StatusTypeDef status;

  HAL_Delay(100);

  status = HAL_I2C_IsDeviceReady(hi2c, devAddr, 3, 1000);
  printf("IsReady: status=%d, error=0x%08lX, state=0x%02lX\r\n", status,
         HAL_I2C_GetError(hi2c), HAL_I2C_GetState(hi2c));

  status = HAL_I2C_Master_Transmit(hi2c, devAddr, &reg, 1, 1000);
  printf("WriteRegAddr: status=%d, error=0x%08lX, state=0x%02lX\r\n", status,
         HAL_I2C_GetError(hi2c), HAL_I2C_GetState(hi2c));

  status = HAL_I2C_Master_Receive(hi2c, devAddr, &value, 1, 1000);
  printf("ReadData: status=%d, error=0x%08lX, state=0x%02lX, value=0x%02X\r\n",
         status, HAL_I2C_GetError(hi2c), HAL_I2C_GetState(hi2c), value);

  value = 0;

  status = HAL_I2C_Mem_Read(hi2c, devAddr, 0x00, I2C_MEMADD_SIZE_8BIT, &value,
                            1, 1000);

  printf("MemRead: status=%d, error=0x%08lX, state=0x%02lX, value=0x%02X\r\n",
         status, HAL_I2C_GetError(hi2c), HAL_I2C_GetState(hi2c), value);
}

void ES8388_DebugDetect(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint16_t addr10 = 0x10 << 1;
    uint16_t addr11 = 0x11 << 1;

    printf("Before detect: state=0x%02lX, error=0x%08lX\r\n",
           (unsigned long)HAL_I2C_GetState(hi2c),
           (unsigned long)HAL_I2C_GetError(hi2c));

    status = HAL_I2C_IsDeviceReady(hi2c, addr10, 3, 1000);
    printf("Probe 0x10: status=%d, state=0x%02lX, error=0x%08lX\r\n",
           status,
           (unsigned long)HAL_I2C_GetState(hi2c),
           (unsigned long)HAL_I2C_GetError(hi2c));

    if (status == HAL_OK) {
        printf("0x10 is ready, skip 0x11\r\n");
        return;
    }

    status = HAL_I2C_IsDeviceReady(hi2c, addr11, 3, 1000);
    printf("Probe 0x11: status=%d, state=0x%02lX, error=0x%08lX\r\n",
           status,
           (unsigned long)HAL_I2C_GetState(hi2c),
           (unsigned long)HAL_I2C_GetError(hi2c));

    printf("After detect: state=0x%02lX, error=0x%08lX\r\n",
           (unsigned long)HAL_I2C_GetState(hi2c),
           (unsigned long)HAL_I2C_GetError(hi2c));
}
