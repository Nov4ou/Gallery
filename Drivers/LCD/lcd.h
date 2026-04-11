#ifndef __LCD_H
#define __LCD_H

#include "main.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LCD address structure */
typedef struct {
  volatile uint16_t LCD_REG;
  volatile uint16_t LCD_RAM;
} LCD_TypeDef;

/*
 * FSMC config:
 *   NE4  -> LCD_CS
 *   A6   -> LCD_RS
 *   16bit data bus
 *
 * LCD->LCD_REG : RS = 0
 * LCD->LCD_RAM : RS = 1
 */
#define LCD_BASE_ADDR ((uint32_t)(0x6C000000U | (((1U << 6) * 2U) - 2U)))
#define LCD ((LCD_TypeDef *)LCD_BASE_ADDR)

/* Replace these with your real board pins if needed */
#define LCD_BL_PORT GPIOB
#define LCD_BL_PIN GPIO_PIN_1

#define LCD_USE_RST 0
#define LCD_RST_PORT GPIOB
#define LCD_RST_PIN GPIO_PIN_0

#define LCD_WIDTH 480U
#define LCD_HEIGHT 800U

#define LCD_COLOR_WHITE 0xFFFFU
#define LCD_COLOR_BLACK 0x0000U
#define LCD_COLOR_RED 0xF800U
#define LCD_COLOR_GREEN 0x07E0U
#define LCD_COLOR_BLUE 0x001FU

void LCD_BacklightOn(void);
void LCD_BacklightOff(void);
void LCD_Reset(void);

void LCD_WriteRegNo(uint16_t regno);
void LCD_WriteData(uint16_t data);
uint16_t LCD_ReadData(void);
void LCD_WriteReg(uint16_t reg, uint16_t data);

uint16_t LCD_ReadID(void);
void LCD_NT35510_InitRegs(void);
void LCD_Init(void);

void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_WriteColor(uint16_t color);
void LCD_FillRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                  uint16_t color);
void LCD_Clear(uint16_t color);

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif