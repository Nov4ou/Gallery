#ifndef __LCD_H
#define __LCD_H

#include "main.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LCD address structure */
typedef struct
{
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
#define LCD           ((LCD_TypeDef *)LCD_BASE_ADDR)

/* Replace these with your real board pins if needed */
#define LCD_BL_PORT   GPIOB
#define LCD_BL_PIN    GPIO_PIN_1

#define LCD_USE_RST   0
#define LCD_RST_PORT  GPIOB
#define LCD_RST_PIN   GPIO_PIN_0

void LCD_BacklightOn(void);
void LCD_BacklightOff(void);
void LCD_Reset(void);

void LCD_WriteRegNo(uint16_t regno);
void LCD_WriteData(uint16_t data);
uint16_t LCD_ReadData(void);
void LCD_WriteReg(uint16_t reg, uint16_t data);

uint16_t LCD_ReadID(void);

#ifdef __cplusplus
}
#endif

#endif