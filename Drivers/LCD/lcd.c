#include "lcd.h"

static void lcdOptDelay(volatile uint32_t n)
{
    while (n--)
    {
        __NOP();
    }
}

void LCD_BacklightOn(void)
{
    // HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_SET);
}

void LCD_BacklightOff(void)
{
    // HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_RESET);
}

void LCD_Reset(void)
{
#if LCD_USE_RST
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(120);
#else
    HAL_Delay(20);
#endif
}

void LCD_WriteRegNo(uint16_t regno)
{
    regno = regno;      /* Keep same style as sample */
    LCD->LCD_REG = regno;
}

void LCD_WriteData(uint16_t data)
{
    LCD->LCD_RAM = data;
}

uint16_t LCD_ReadData(void)
{
    volatile uint16_t ram;

    lcdOptDelay(2);
    ram = LCD->LCD_RAM;
    return ram;
}

void LCD_WriteReg(uint16_t reg, uint16_t data)
{
    LCD_WriteRegNo(reg);
    LCD_WriteData(data);
}

uint16_t LCD_ReadID(void)
{
    uint16_t id;

    /*
     * NT35510 read sequence from sample:
     * 1. Send unlock key
     * 2. Read 0xC500 and 0xC501
     * 3. Combine to 0x5510
     */
    LCD_WriteReg(0xF000, 0x0055);
    LCD_WriteReg(0xF001, 0x00AA);
    LCD_WriteReg(0xF002, 0x0052);
    LCD_WriteReg(0xF003, 0x0008);
    LCD_WriteReg(0xF004, 0x0001);

    LCD_WriteRegNo(0xC500);
    id = LCD_ReadData();
    id <<= 8;

    LCD_WriteRegNo(0xC501);
    id |= LCD_ReadData();

    HAL_Delay(5);

    return id;
}