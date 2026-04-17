/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "album.h"
#include "bmp.h"
#include "gt911.h"
#include "image_scale.h"
#include "image_upload.h"
#include "jpeg_viewer.h"
#include "lcd.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lvgl.h"
#include "mw8266.h"
#include "photo_view.h"
#include "stdint.h"
#include "string.h"
#include "ui_main.h"
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MW8266_RX_DMA_BUF_SIZE 4096U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SD_HandleTypeDef hsd;

TIM_HandleTypeDef htim12;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;

SRAM_HandleTypeDef hsram1;
SRAM_HandleTypeDef hsram2;

/* USER CODE BEGIN PV */
FATFS gFatFs;
GT911_State_t tpState;
uint8_t lastPressed = 0;

volatile uint32_t gLvTickCounter = 0;

uint16_t *jpgBuf = JPG_DECODE_BUF;
uint16_t *photoBuf = PHOTO_DISPLAY_BUF;

Mw8266Handle_t mw8266;
uint8_t mw8266RxDmaBuf[MW8266_RX_DMA_BUF_SIZE];

ImageUploadContext_t uploadCtx;

static uint32_t gLastPhotoScanTick = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM12_Init(void);
static void MX_FSMC_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
static void FormatSdCard(void);
static void CreateTestFile(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch) {
  uint8_t c = (uint8_t)ch;
  HAL_UART_Transmit(&huart1, &c, 1, HAL_MAX_DELAY);
  return ch;
}

static int ExtractIpdPayload(uint8_t *rxBuf, uint8_t **payload,
                             uint16_t *payloadLen, uint8_t *linkId) {
  char *ipd;
  char *colon;
  unsigned int id = 0;
  unsigned int len = 0;

  if (rxBuf == NULL || payload == NULL || payloadLen == NULL ||
      linkId == NULL) {
    return -1;
  }

  ipd = strstr((char *)rxBuf, "+IPD,");
  if (ipd == NULL) {
    return -2;
  }

  if (sscanf(ipd, "+IPD,%u,%u", &id, &len) != 2) {
    return -3;
  }

  colon = strchr(ipd, ':');
  if (colon == NULL) {
    return -4;
  }

  *linkId = (uint8_t)id;
  *payloadLen = (uint16_t)len;
  *payload = (uint8_t *)(colon + 1);

  return 0;
}

static int ParseImageHeader(const char *payload, uint32_t *fileSize,
                            char *fileName, uint16_t fileNameSize) {
  unsigned long size;
  char name[64];

  if (payload == NULL || fileSize == NULL || fileName == NULL) {
    return -1;
  }

  if (sscanf(payload, "IMG:%lu:%63s", &size, name) != 2) {
    return -2;
  }

  *fileSize = (uint32_t)size;
  strncpy(fileName, name, fileNameSize - 1);
  fileName[fileNameSize - 1] = '\0';

  return 0;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */
  FRESULT res;
  uint16_t lcdId;
  int bmpRet;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_USART1_UART_Init();
  MX_TIM12_Init();
  MX_FSMC_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  // HAL_Delay(1000);
  // FormatSdCard();
  // CreateTestFile();

  HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
  lcdId = LCD_ReadID();
  LCD_Init();
  LCD_Clear(LCD_COLOR_BLACK);
  res = f_mount(&gFatFs, "", 1);
  if (res == FR_OK) {
    bmpRet = BMP_ShowFromSD("0:/hello.bmp", 0, 0);
    (void)bmpRet;
    if (Album_ScanPhotos("0:/") == 0) {
      printf("Photo scan success, total=%lu\r\n", (unsigned long)gPhotoCount);
    } else {
      printf("Photo scan failed\r\n");
    }
  } else {
    printf("f_mount failed: %d\r\n", res);
  }
  Album_ScanPhotos("0:/");

  GT911_Init();
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();
  ui_main_init();

  // uint16_t xMax, yMax;
  // if (GT911_ReadResolution(&xMax, &yMax) == 0) {
  //   printf("GT911 resolution: xMax=%u yMax=%u\r\n", xMax, yMax);
  // } else {
  //   printf("Read GT911 resolution failed\r\n");
  // }

  // uint8_t b0, b1, b2, b3;

  // if (GT911_ReadOneByte(0x8048, &b0) == 0 &&
  //     GT911_ReadOneByte(0x8049, &b1) == 0 &&
  //     GT911_ReadOneByte(0x804A, &b2) == 0 &&
  //     GT911_ReadOneByte(0x804B, &b3) == 0) {
  //   printf("8048=%02X 8049=%02X 804A=%02X 804B=%02X\r\n", b0, b1, b2, b3);
  // } else {
  //   printf("Read one-byte test failed\r\n");
  // }

  // uint16_t *sram = (uint16_t *)0x68000000U;
  // sram[0] = 0x1234;
  // sram[1] = 0xABCD;
  // sram[2] = 0x55AA;
  // printf("SRAM: %04X %04X %04X\r\n", sram[0], sram[1], sram[2]);

  Mw8266_Init(&mw8266, &huart3, mw8266RxDmaBuf, sizeof(mw8266RxDmaBuf));
  // Mw8266_StartReceive(&mw8266);
  printf("MW8266 DMA+IDLE started\r\n");

  if (Mw8266_TestAT(&mw8266) == mw8266Ok) {
    printf("MW8266 AT OK\r\n");
  } else {
    printf("MW8266 AT FAIL\r\n");
  }

  Mw8266_SetEcho(&mw8266, 0);

  /* 1. Set AP mode */
  if (Mw8266_SetWifiMode(&mw8266, 2) == mw8266Ok) {
    printf("Set AP mode OK\r\n");
  } else {
    printf("Set AP mode FAIL\r\n");
  }

  /* 2. Configure hotspot: SSID / password / channel / encryption */
  if (Mw8266_ConfigAP(&mw8266, "STM32_AP", "12345678", 1, 3) == mw8266Ok) {
    printf("Config AP OK\r\n");
  } else {
    printf("Config AP FAIL\r\n");
  }

  /* 3. Enable multi connection */
  if (Mw8266_SetMux(&mw8266, 1) == mw8266Ok) {
    printf("Set MUX OK\r\n");
  } else {
    printf("Set MUX FAIL\r\n");
  }

  /* 4. Start TCP server on port 8080 */
  if (Mw8266_StartServer(&mw8266, 8080) == mw8266Ok) {
    printf("Start server OK\r\n");
  } else {
    printf("Start server FAIL\r\n");
  }

  /* 5. Query IP */
  Mw8266_GetIP(&mw8266);
  printf("IP info:\r\n%s\r\n", mw8266.rxDmaBuf);

  Album_Init();
  uint32_t lastPrint = HAL_GetTick();

  ImageUpload_Init(&uploadCtx);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // PhotoScanTask();

    if (mw8266.rxFrameReady) {
      uint8_t linkId;
      uint16_t payloadLen;
      uint8_t *payload;
      int parseRet;

      mw8266.rxFrameReady = 0;

      printf("MW8266 RX frame len = %u\r\n", mw8266.rxFrameLen);

      if (mw8266.rxFrameLen < mw8266.rxDmaBufSize) {
        mw8266.rxDmaBuf[mw8266.rxFrameLen] = '\0';
      } else {
        mw8266.rxDmaBuf[mw8266.rxDmaBufSize - 1U] = '\0';
      }

      // printf("RX frame:\r\n%s\r\n", mw8266.rxDmaBuf);

      /* 1. Connection state frames */
      if (strstr((char *)mw8266.rxDmaBuf, ",CONNECT") != NULL) {
        printf("Client connected\r\n");
      } else if (strstr((char *)mw8266.rxDmaBuf, ",CLOSED") != NULL) {
        printf("Client disconnected\r\n");
      } else {
        /* 2. Try parse +IPD frame */
        parseRet = Mw8266_ParseIpd(mw8266.rxDmaBuf, mw8266.rxFrameLen, &linkId,
                                   &payloadLen, &payload);

        if (parseRet == 0) {
          uint16_t actualAvailableLen;
          int completeRet;

          completeRet =
              Mw8266_IsIpdComplete(mw8266.rxDmaBuf, mw8266.rxFrameLen, payload,
                                   payloadLen, &actualAvailableLen);

          printf("payloadLen=%u actualAvailableLen=%u completeRet=%d\r\n",
                 payloadLen, actualAvailableLen, completeRet);

          if (completeRet == 1) {
            /* 2.1 Upload header */
            if (uploadCtx.state == UploadIdle) {
              if (ImageUpload_ParseHexHeader(&uploadCtx, (char *)payload) ==
                  0) {
                int startRet = ImageUpload_StartHexFile(&uploadCtx);
                printf("ImageUpload_StartHexFile ret = %d\r\n", startRet);

                if (startRet == 0) {
                  const char reply[] = "HEX HEADER OK\r\n";

                  Mw8266_SendDataToClient(&mw8266, linkId,
                                          (const uint8_t *)reply,
                                          sizeof(reply) - 1U, 3000);

                  printf("Ready to receive HEX image data\r\n");
                }
              } else {
                /* Not an upload header, just print payload */
                printf("Received payload from link %u:\r\n", linkId);
                for (uint16_t i = 0; i < payloadLen; i++) {
                  printf("%c", payload[i]);
                }
                printf("\r\n");
              }
            }
            /* 2.2 Upload data body */
            else if (uploadCtx.state == UploadReceivingHexData) {
              int wrRet;

              wrRet =
                  ImageUpload_WriteHexStream(&uploadCtx, payload, payloadLen);

              printf("ImageUpload_WriteHexStream ret = %d, written=%lu/%lu\r\n",
                     wrRet, (unsigned long)uploadCtx.writtenBinarySize,
                     (unsigned long)uploadCtx.expectedBinarySize);

              if (ImageUpload_IsComplete(&uploadCtx)) {
                ImageUpload_Close(&uploadCtx);
                Album_ScanPhotos("0:/");
                Album_OpenUploadedPhoto(uploadCtx.fileName);
                printf("HEX image upload complete: %s\r\n", uploadCtx.fileName);

                /* 可选：这里可以加 JPG 头尾检查 */
                /* DebugCheckUploadedJpg(uploadCtx.fileName); */
              }
            } else {
              printf("Unhandled upload state = %d\r\n", uploadCtx.state);
            }
          } else {
            printf("IPD not complete yet\r\n");
          }
        } else {
          printf("Unknown frame\r\n");
        }
      }

      /* Restart DMA reception for next frame */
      Mw8266_StartReceive(&mw8266);
    }

    lv_timer_handler();
    HAL_Delay(5);
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief SDIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_SDIO_SD_Init(void) {

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd.Init.ClockDiv = 2;
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */
}

/**
 * @brief TIM12 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM12_Init(void) {

  /* USER CODE BEGIN TIM12_Init 0 */

  /* USER CODE END TIM12_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM12_Init 1 */

  /* USER CODE END TIM12_Init 1 */
  htim12.Instance = TIM12;
  htim12.Init.Prescaler = 83;
  htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim12.Init.Period = 99;
  htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim12.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim12) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim12) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 100;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM12_Init 2 */

  /* USER CODE END TIM12_Init 2 */
  HAL_TIM_MspPostInit(&htim12);
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void) {

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GT911_RST_GPIO_Port, GT911_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, LED0_Pin | GT911_SDA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GT911_SCL_Pin | GT911_INT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : GT911_RST_Pin */
  GPIO_InitStruct.Pin = GT911_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GT911_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED0_Pin */
  GPIO_InitStruct.Pin = LED0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GT911_SCL_Pin */
  GPIO_InitStruct.Pin = GT911_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GT911_SCL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GT911_INT_Pin */
  GPIO_InitStruct.Pin = GT911_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GT911_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GT911_SDA_Pin */
  GPIO_InitStruct.Pin = GT911_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GT911_SDA_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* FSMC initialization function */
static void MX_FSMC_Init(void) {

  /* USER CODE BEGIN FSMC_Init 0 */

  /* USER CODE END FSMC_Init 0 */

  FSMC_NORSRAM_TimingTypeDef Timing = {0};
  FSMC_NORSRAM_TimingTypeDef ExtTiming = {0};

  /* USER CODE BEGIN FSMC_Init 1 */

  /* USER CODE END FSMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
   */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK4;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 60;
  Timing.BusTurnAroundDuration = 0;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */
  ExtTiming.AddressSetupTime = 9;
  ExtTiming.AddressHoldTime = 15;
  ExtTiming.DataSetupTime = 9;
  ExtTiming.BusTurnAroundDuration = 0;
  ExtTiming.CLKDivision = 16;
  ExtTiming.DataLatency = 17;
  ExtTiming.AccessMode = FSMC_ACCESS_MODE_A;

  if (HAL_SRAM_Init(&hsram1, &Timing, &ExtTiming) != HAL_OK) {
    Error_Handler();
  }

  /** Perform the SRAM2 memory initialization sequence
   */
  hsram2.Instance = FSMC_NORSRAM_DEVICE;
  hsram2.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram2.Init */
  hsram2.Init.NSBank = FSMC_NORSRAM_BANK3;
  hsram2.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram2.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram2.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram2.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram2.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram2.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram2.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram2.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram2.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram2.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram2.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram2.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram2.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 2;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 6;
  Timing.BusTurnAroundDuration = 1;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram2, &Timing, NULL) != HAL_OK) {
    Error_Handler();
  }

  /* USER CODE BEGIN FSMC_Init 2 */

  /* USER CODE END FSMC_Init 2 */
}

/* USER CODE BEGIN 4 */
static void FormatSdCard(void) {
  FRESULT res;
  BYTE workBuffer[_MAX_SS];

  printf("Start formatting SD card...\r\n");

  /* Optional: mount first */
  res = f_mount(&SDFatFS, (TCHAR const *)SDPath, 0);
  if (res != FR_OK) {
    printf("f_mount failed: %d\r\n", res);
    return;
  }

  // uint8_t readBuf[512];

  // if (HAL_SD_ReadBlocks(&hsd, readBuf, 0, 1, 1000) != HAL_OK) {
  //   printf("HAL_SD_ReadBlocks failed, err = 0x%08lX\r\n", hsd.ErrorCode);
  //   Error_Handler();
  // }

  printf("HAL_SD_ReadBlocks success\r\n");

  /* Make filesystem */
  res =
      f_mkfs((TCHAR const *)SDPath, FM_ANY, 0, workBuffer, sizeof(workBuffer));
  if (res != FR_OK) {
    printf("f_mkfs failed: %d\r\n", res);
    return;
  }

  printf("f_mkfs success\r\n");

  /* Unmount */
  res = f_mount(NULL, (TCHAR const *)SDPath, 0);
  if (res != FR_OK) {
    printf("unmount failed: %d\r\n", res);
    return;
  }

  /* Mount again */
  res = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1);
  if (res != FR_OK) {
    printf("mount after mkfs failed: %d\r\n", res);
    return;
  }

  printf("SD card formatted and mounted successfully\r\n");
}

static void CreateTestFile(void) {
  FRESULT res;
  UINT bytesWritten;
  char writeBuffer[] = "Hello from STM32 FatFs!\r\n";

  printf("Creating test file...\r\n");

  res = f_open(&SDFile, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
  if (res != FR_OK) {
    printf("f_open failed: %d\r\n", res);
    return;
  }

  res = f_write(&SDFile, writeBuffer, sizeof(writeBuffer) - 1, &bytesWritten);
  if (res != FR_OK) {
    printf("f_write failed: %d\r\n", res);
    f_close(&SDFile);
    return;
  }

  printf("Write success, bytesWritten = %u\r\n", bytesWritten);

  res = f_close(&SDFile);
  if (res != FR_OK) {
    printf("f_close failed: %d\r\n", res);
    return;
  }

  printf("File created successfully\r\n");
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
