#ifndef __GT911_H
#define __GT911_H

#include "main.h"
#include <stdint.h>

#define LCD_WIDTH 480U
#define LCD_HEIGHT 800U

typedef struct {
  uint16_t x;
  uint16_t y;
  uint8_t pressed;
} GT911_State_t;

uint8_t GT911_Init(void);
uint8_t GT911_ReadResolution(uint16_t *xMax, uint16_t *yMax);
uint8_t GT911_ReadOneByte(uint16_t reg, uint8_t *value);
uint8_t GT911_ReadState(GT911_State_t *state);

#endif