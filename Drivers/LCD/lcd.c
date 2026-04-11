#include "lcd.h"

static void lcdOptDelay(volatile uint32_t n) {
  while (n--) {
    __NOP();
  }
}

void LCD_BacklightOn(void) {
  // HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_SET);
}

void LCD_BacklightOff(void) {
  // HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_RESET);
}

void LCD_Reset(void) {
#if LCD_USE_RST
  HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
  HAL_Delay(120);
#else
  HAL_Delay(20);
#endif
}

void LCD_WriteRegNo(uint16_t regno) {
  regno = regno; /* Keep same style as sample */
  LCD->LCD_REG = regno;
}

void LCD_WriteData(uint16_t data) { LCD->LCD_RAM = data; }

uint16_t LCD_ReadData(void) {
  volatile uint16_t ram;

  lcdOptDelay(2);
  ram = LCD->LCD_RAM;
  return ram;
}

void LCD_WriteReg(uint16_t reg, uint16_t data) {
  LCD_WriteRegNo(reg);
  LCD_WriteData(data);
}

uint16_t LCD_ReadID(void) {
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

void LCD_NT35510_InitRegs(void) {
  LCD_WriteReg(0xF000, 0x0055);
  LCD_WriteReg(0xF001, 0x00AA);
  LCD_WriteReg(0xF002, 0x0052);
  LCD_WriteReg(0xF003, 0x0008);
  LCD_WriteReg(0xF004, 0x0001);

  /* AVDD ratio */
  LCD_WriteReg(0xB600, 0x0034);
  LCD_WriteReg(0xB601, 0x0034);
  LCD_WriteReg(0xB602, 0x0034);

  /* AVDD 5.2V */
  LCD_WriteReg(0xB000, 0x000D);
  LCD_WriteReg(0xB001, 0x000D);
  LCD_WriteReg(0xB002, 0x000D);

  /* AVEE ratio */
  LCD_WriteReg(0xB700, 0x0024);
  LCD_WriteReg(0xB701, 0x0024);
  LCD_WriteReg(0xB702, 0x0024);

  /* AVEE -5.2V */
  LCD_WriteReg(0xB100, 0x000D);
  LCD_WriteReg(0xB101, 0x000D);
  LCD_WriteReg(0xB102, 0x000D);

  /* VCL */
  LCD_WriteReg(0xB800, 0x0024);
  LCD_WriteReg(0xB801, 0x0024);
  LCD_WriteReg(0xB802, 0x0024);

  LCD_WriteReg(0xB200, 0x0000);

  /* VGH ratio */
  LCD_WriteReg(0xB900, 0x0024);
  LCD_WriteReg(0xB901, 0x0024);
  LCD_WriteReg(0xB902, 0x0024);

  /* VGH 10.2V */
  LCD_WriteReg(0xB300, 0x0005);
  LCD_WriteReg(0xB301, 0x0005);
  LCD_WriteReg(0xB302, 0x0005);

  /* VGLX ratio */
  LCD_WriteReg(0xBA00, 0x0034);
  LCD_WriteReg(0xBA01, 0x0034);
  LCD_WriteReg(0xBA02, 0x0034);

  /* VGL_REG -12.6V */
  LCD_WriteReg(0xB500, 0x000B);
  LCD_WriteReg(0xB501, 0x000B);
  LCD_WriteReg(0xB502, 0x000B);

  /* VGMP/VGSP 4.5V */
  LCD_WriteReg(0xBC00, 0x0000);
  LCD_WriteReg(0xBC01, 0x00A3);
  LCD_WriteReg(0xBC02, 0x0000);

  /* GMN/VGSN -4.5V */
  LCD_WriteReg(0xBD00, 0x0000);
  LCD_WriteReg(0xBD01, 0x00A3);
  LCD_WriteReg(0xBD02, 0x0000);

  /* VCOM -1.25V */
  LCD_WriteReg(0xBE00, 0x0000);
  LCD_WriteReg(0xBE01, 0x0037);

  /* Gamma Setting */
  LCD_WriteReg(0xD100, 0x0000);
  LCD_WriteReg(0xD101, 0x0037);
  LCD_WriteReg(0xD102, 0x0000);
  LCD_WriteReg(0xD103, 0x0053);
  LCD_WriteReg(0xD104, 0x0000);
  LCD_WriteReg(0xD105, 0x0079);
  LCD_WriteReg(0xD106, 0x0000);
  LCD_WriteReg(0xD107, 0x0097);
  LCD_WriteReg(0xD108, 0x0000);
  LCD_WriteReg(0xD109, 0x00B1);
  LCD_WriteReg(0xD10A, 0x0000);
  LCD_WriteReg(0xD10B, 0x00D5);
  LCD_WriteReg(0xD10C, 0x0000);
  LCD_WriteReg(0xD10D, 0x00F4);
  LCD_WriteReg(0xD10E, 0x0001);
  LCD_WriteReg(0xD10F, 0x0023);
  LCD_WriteReg(0xD110, 0x0001);
  LCD_WriteReg(0xD111, 0x0049);
  LCD_WriteReg(0xD112, 0x0001);
  LCD_WriteReg(0xD113, 0x0087);
  LCD_WriteReg(0xD114, 0x0001);
  LCD_WriteReg(0xD115, 0x00B6);
  LCD_WriteReg(0xD116, 0x0002);
  LCD_WriteReg(0xD117, 0x0000);
  LCD_WriteReg(0xD118, 0x0002);
  LCD_WriteReg(0xD119, 0x003B);
  LCD_WriteReg(0xD11A, 0x0002);
  LCD_WriteReg(0xD11B, 0x003D);
  LCD_WriteReg(0xD11C, 0x0002);
  LCD_WriteReg(0xD11D, 0x0075);
  LCD_WriteReg(0xD11E, 0x0002);
  LCD_WriteReg(0xD11F, 0x00B1);
  LCD_WriteReg(0xD120, 0x0002);
  LCD_WriteReg(0xD121, 0x00D5);
  LCD_WriteReg(0xD122, 0x0003);
  LCD_WriteReg(0xD123, 0x0009);
  LCD_WriteReg(0xD124, 0x0003);
  LCD_WriteReg(0xD125, 0x0028);
  LCD_WriteReg(0xD126, 0x0003);
  LCD_WriteReg(0xD127, 0x0052);
  LCD_WriteReg(0xD128, 0x0003);
  LCD_WriteReg(0xD129, 0x006B);
  LCD_WriteReg(0xD12A, 0x0003);
  LCD_WriteReg(0xD12B, 0x008D);
  LCD_WriteReg(0xD12C, 0x0003);
  LCD_WriteReg(0xD12D, 0x00A2);
  LCD_WriteReg(0xD12E, 0x0003);
  LCD_WriteReg(0xD12F, 0x00BB);
  LCD_WriteReg(0xD130, 0x0003);
  LCD_WriteReg(0xD131, 0x00C1);
  LCD_WriteReg(0xD132, 0x0003);
  LCD_WriteReg(0xD133, 0x00C1);

  LCD_WriteReg(0xD200, 0x0000);
  LCD_WriteReg(0xD201, 0x0037);
  LCD_WriteReg(0xD202, 0x0000);
  LCD_WriteReg(0xD203, 0x0053);
  LCD_WriteReg(0xD204, 0x0000);
  LCD_WriteReg(0xD205, 0x0079);
  LCD_WriteReg(0xD206, 0x0000);
  LCD_WriteReg(0xD207, 0x0097);
  LCD_WriteReg(0xD208, 0x0000);
  LCD_WriteReg(0xD209, 0x00B1);
  LCD_WriteReg(0xD20A, 0x0000);
  LCD_WriteReg(0xD20B, 0x00D5);
  LCD_WriteReg(0xD20C, 0x0000);
  LCD_WriteReg(0xD20D, 0x00F4);
  LCD_WriteReg(0xD20E, 0x0001);
  LCD_WriteReg(0xD20F, 0x0023);
  LCD_WriteReg(0xD210, 0x0001);
  LCD_WriteReg(0xD211, 0x0049);
  LCD_WriteReg(0xD212, 0x0001);
  LCD_WriteReg(0xD213, 0x0087);
  LCD_WriteReg(0xD214, 0x0001);
  LCD_WriteReg(0xD215, 0x00B6);
  LCD_WriteReg(0xD216, 0x0002);
  LCD_WriteReg(0xD217, 0x0000);
  LCD_WriteReg(0xD218, 0x0002);
  LCD_WriteReg(0xD219, 0x003B);
  LCD_WriteReg(0xD21A, 0x0002);
  LCD_WriteReg(0xD21B, 0x003D);
  LCD_WriteReg(0xD21C, 0x0002);
  LCD_WriteReg(0xD21D, 0x0075);
  LCD_WriteReg(0xD21E, 0x0002);
  LCD_WriteReg(0xD21F, 0x00B1);
  LCD_WriteReg(0xD220, 0x0002);
  LCD_WriteReg(0xD221, 0x00D5);
  LCD_WriteReg(0xD222, 0x0003);
  LCD_WriteReg(0xD223, 0x0009);
  LCD_WriteReg(0xD224, 0x0003);
  LCD_WriteReg(0xD225, 0x0028);
  LCD_WriteReg(0xD226, 0x0003);
  LCD_WriteReg(0xD227, 0x0052);
  LCD_WriteReg(0xD228, 0x0003);
  LCD_WriteReg(0xD229, 0x006B);
  LCD_WriteReg(0xD22A, 0x0003);
  LCD_WriteReg(0xD22B, 0x008D);
  LCD_WriteReg(0xD22C, 0x0003);
  LCD_WriteReg(0xD22D, 0x00A2);
  LCD_WriteReg(0xD22E, 0x0003);
  LCD_WriteReg(0xD22F, 0x00BB);
  LCD_WriteReg(0xD230, 0x0003);
  LCD_WriteReg(0xD231, 0x00C1);
  LCD_WriteReg(0xD232, 0x0003);
  LCD_WriteReg(0xD233, 0x00C1);

  LCD_WriteReg(0xD300, 0x0000);
  LCD_WriteReg(0xD301, 0x0037);
  LCD_WriteReg(0xD302, 0x0000);
  LCD_WriteReg(0xD303, 0x0053);
  LCD_WriteReg(0xD304, 0x0000);
  LCD_WriteReg(0xD305, 0x0079);
  LCD_WriteReg(0xD306, 0x0000);
  LCD_WriteReg(0xD307, 0x0097);
  LCD_WriteReg(0xD308, 0x0000);
  LCD_WriteReg(0xD309, 0x00B1);
  LCD_WriteReg(0xD30A, 0x0000);
  LCD_WriteReg(0xD30B, 0x00D5);
  LCD_WriteReg(0xD30C, 0x0000);
  LCD_WriteReg(0xD30D, 0x00F4);
  LCD_WriteReg(0xD30E, 0x0001);
  LCD_WriteReg(0xD30F, 0x0023);
  LCD_WriteReg(0xD310, 0x0001);
  LCD_WriteReg(0xD311, 0x0049);
  LCD_WriteReg(0xD312, 0x0001);
  LCD_WriteReg(0xD313, 0x0087);
  LCD_WriteReg(0xD314, 0x0001);
  LCD_WriteReg(0xD315, 0x00B6);
  LCD_WriteReg(0xD316, 0x0002);
  LCD_WriteReg(0xD317, 0x0000);
  LCD_WriteReg(0xD318, 0x0002);
  LCD_WriteReg(0xD319, 0x003B);
  LCD_WriteReg(0xD31A, 0x0002);
  LCD_WriteReg(0xD31B, 0x003D);
  LCD_WriteReg(0xD31C, 0x0002);
  LCD_WriteReg(0xD31D, 0x0075);
  LCD_WriteReg(0xD31E, 0x0002);
  LCD_WriteReg(0xD31F, 0x00B1);
  LCD_WriteReg(0xD320, 0x0002);
  LCD_WriteReg(0xD321, 0x00D5);
  LCD_WriteReg(0xD322, 0x0003);
  LCD_WriteReg(0xD323, 0x0009);
  LCD_WriteReg(0xD324, 0x0003);
  LCD_WriteReg(0xD325, 0x0028);
  LCD_WriteReg(0xD326, 0x0003);
  LCD_WriteReg(0xD327, 0x0052);
  LCD_WriteReg(0xD328, 0x0003);
  LCD_WriteReg(0xD329, 0x006B);
  LCD_WriteReg(0xD32A, 0x0003);
  LCD_WriteReg(0xD32B, 0x008D);
  LCD_WriteReg(0xD32C, 0x0003);
  LCD_WriteReg(0xD32D, 0x00A2);
  LCD_WriteReg(0xD32E, 0x0003);
  LCD_WriteReg(0xD32F, 0x00BB);
  LCD_WriteReg(0xD330, 0x0003);
  LCD_WriteReg(0xD331, 0x00C1);
  LCD_WriteReg(0xD332, 0x0003);
  LCD_WriteReg(0xD333, 0x00C1);

  LCD_WriteReg(0xD400, 0x0000);
  LCD_WriteReg(0xD401, 0x0037);
  LCD_WriteReg(0xD402, 0x0000);
  LCD_WriteReg(0xD403, 0x0053);
  LCD_WriteReg(0xD404, 0x0000);
  LCD_WriteReg(0xD405, 0x0079);
  LCD_WriteReg(0xD406, 0x0000);
  LCD_WriteReg(0xD407, 0x0097);
  LCD_WriteReg(0xD408, 0x0000);
  LCD_WriteReg(0xD409, 0x00B1);
  LCD_WriteReg(0xD40A, 0x0000);
  LCD_WriteReg(0xD40B, 0x00D5);
  LCD_WriteReg(0xD40C, 0x0000);
  LCD_WriteReg(0xD40D, 0x00F4);
  LCD_WriteReg(0xD40E, 0x0001);
  LCD_WriteReg(0xD40F, 0x0023);
  LCD_WriteReg(0xD410, 0x0001);
  LCD_WriteReg(0xD411, 0x0049);
  LCD_WriteReg(0xD412, 0x0001);
  LCD_WriteReg(0xD413, 0x0087);
  LCD_WriteReg(0xD414, 0x0001);
  LCD_WriteReg(0xD415, 0x00B6);
  LCD_WriteReg(0xD416, 0x0002);
  LCD_WriteReg(0xD417, 0x0000);
  LCD_WriteReg(0xD418, 0x0002);
  LCD_WriteReg(0xD419, 0x003B);
  LCD_WriteReg(0xD41A, 0x0002);
  LCD_WriteReg(0xD41B, 0x003D);
  LCD_WriteReg(0xD41C, 0x0002);
  LCD_WriteReg(0xD41D, 0x0075);
  LCD_WriteReg(0xD41E, 0x0002);
  LCD_WriteReg(0xD41F, 0x00B1);
  LCD_WriteReg(0xD420, 0x0002);
  LCD_WriteReg(0xD421, 0x00D5);
  LCD_WriteReg(0xD422, 0x0003);
  LCD_WriteReg(0xD423, 0x0009);
  LCD_WriteReg(0xD424, 0x0003);
  LCD_WriteReg(0xD425, 0x0028);
  LCD_WriteReg(0xD426, 0x0003);
  LCD_WriteReg(0xD427, 0x0052);
  LCD_WriteReg(0xD428, 0x0003);
  LCD_WriteReg(0xD429, 0x006B);
  LCD_WriteReg(0xD42A, 0x0003);
  LCD_WriteReg(0xD42B, 0x008D);
  LCD_WriteReg(0xD42C, 0x0003);
  LCD_WriteReg(0xD42D, 0x00A2);
  LCD_WriteReg(0xD42E, 0x0003);
  LCD_WriteReg(0xD42F, 0x00BB);
  LCD_WriteReg(0xD430, 0x0003);
  LCD_WriteReg(0xD431, 0x00C1);
  LCD_WriteReg(0xD432, 0x0003);
  LCD_WriteReg(0xD433, 0x00C1);

  LCD_WriteReg(0xD500, 0x0000);
  LCD_WriteReg(0xD501, 0x0037);
  LCD_WriteReg(0xD502, 0x0000);
  LCD_WriteReg(0xD503, 0x0053);
  LCD_WriteReg(0xD504, 0x0000);
  LCD_WriteReg(0xD505, 0x0079);
  LCD_WriteReg(0xD506, 0x0000);
  LCD_WriteReg(0xD507, 0x0097);
  LCD_WriteReg(0xD508, 0x0000);
  LCD_WriteReg(0xD509, 0x00B1);
  LCD_WriteReg(0xD50A, 0x0000);
  LCD_WriteReg(0xD50B, 0x00D5);
  LCD_WriteReg(0xD50C, 0x0000);
  LCD_WriteReg(0xD50D, 0x00F4);
  LCD_WriteReg(0xD50E, 0x0001);
  LCD_WriteReg(0xD50F, 0x0023);
  LCD_WriteReg(0xD510, 0x0001);
  LCD_WriteReg(0xD511, 0x0049);
  LCD_WriteReg(0xD512, 0x0001);
  LCD_WriteReg(0xD513, 0x0087);
  LCD_WriteReg(0xD514, 0x0001);
  LCD_WriteReg(0xD515, 0x00B6);
  LCD_WriteReg(0xD516, 0x0002);
  LCD_WriteReg(0xD517, 0x0000);
  LCD_WriteReg(0xD518, 0x0002);
  LCD_WriteReg(0xD519, 0x003B);
  LCD_WriteReg(0xD51A, 0x0002);
  LCD_WriteReg(0xD51B, 0x003D);
  LCD_WriteReg(0xD51C, 0x0002);
  LCD_WriteReg(0xD51D, 0x0075);
  LCD_WriteReg(0xD51E, 0x0002);
  LCD_WriteReg(0xD51F, 0x00B1);
  LCD_WriteReg(0xD520, 0x0002);
  LCD_WriteReg(0xD521, 0x00D5);
  LCD_WriteReg(0xD522, 0x0003);
  LCD_WriteReg(0xD523, 0x0009);
  LCD_WriteReg(0xD524, 0x0003);
  LCD_WriteReg(0xD525, 0x0028);
  LCD_WriteReg(0xD526, 0x0003);
  LCD_WriteReg(0xD527, 0x0052);
  LCD_WriteReg(0xD528, 0x0003);
  LCD_WriteReg(0xD529, 0x006B);
  LCD_WriteReg(0xD52A, 0x0003);
  LCD_WriteReg(0xD52B, 0x008D);
  LCD_WriteReg(0xD52C, 0x0003);
  LCD_WriteReg(0xD52D, 0x00A2);
  LCD_WriteReg(0xD52E, 0x0003);
  LCD_WriteReg(0xD52F, 0x00BB);
  LCD_WriteReg(0xD530, 0x0003);
  LCD_WriteReg(0xD531, 0x00C1);
  LCD_WriteReg(0xD532, 0x0003);
  LCD_WriteReg(0xD533, 0x00C1);

  LCD_WriteReg(0xD600, 0x0000);
  LCD_WriteReg(0xD601, 0x0037);
  LCD_WriteReg(0xD602, 0x0000);
  LCD_WriteReg(0xD603, 0x0053);
  LCD_WriteReg(0xD604, 0x0000);
  LCD_WriteReg(0xD605, 0x0079);
  LCD_WriteReg(0xD606, 0x0000);
  LCD_WriteReg(0xD607, 0x0097);
  LCD_WriteReg(0xD608, 0x0000);
  LCD_WriteReg(0xD609, 0x00B1);
  LCD_WriteReg(0xD60A, 0x0000);
  LCD_WriteReg(0xD60B, 0x00D5);
  LCD_WriteReg(0xD60C, 0x0000);
  LCD_WriteReg(0xD60D, 0x00F4);
  LCD_WriteReg(0xD60E, 0x0001);
  LCD_WriteReg(0xD60F, 0x0023);
  LCD_WriteReg(0xD610, 0x0001);
  LCD_WriteReg(0xD611, 0x0049);
  LCD_WriteReg(0xD612, 0x0001);
  LCD_WriteReg(0xD613, 0x0087);
  LCD_WriteReg(0xD614, 0x0001);
  LCD_WriteReg(0xD615, 0x00B6);
  LCD_WriteReg(0xD616, 0x0002);
  LCD_WriteReg(0xD617, 0x0000);
  LCD_WriteReg(0xD618, 0x0002);
  LCD_WriteReg(0xD619, 0x003B);
  LCD_WriteReg(0xD61A, 0x0002);
  LCD_WriteReg(0xD61B, 0x003D);
  LCD_WriteReg(0xD61C, 0x0002);
  LCD_WriteReg(0xD61D, 0x0075);
  LCD_WriteReg(0xD61E, 0x0002);
  LCD_WriteReg(0xD61F, 0x00B1);
  LCD_WriteReg(0xD620, 0x0002);
  LCD_WriteReg(0xD621, 0x00D5);
  LCD_WriteReg(0xD622, 0x0003);
  LCD_WriteReg(0xD623, 0x0009);
  LCD_WriteReg(0xD624, 0x0003);
  LCD_WriteReg(0xD625, 0x0028);
  LCD_WriteReg(0xD626, 0x0003);
  LCD_WriteReg(0xD627, 0x0052);
  LCD_WriteReg(0xD628, 0x0003);
  LCD_WriteReg(0xD629, 0x006B);
  LCD_WriteReg(0xD62A, 0x0003);
  LCD_WriteReg(0xD62B, 0x008D);
  LCD_WriteReg(0xD62C, 0x0003);
  LCD_WriteReg(0xD62D, 0x00A2);
  LCD_WriteReg(0xD62E, 0x0003);
  LCD_WriteReg(0xD62F, 0x00BB);
  LCD_WriteReg(0xD630, 0x0003);
  LCD_WriteReg(0xD631, 0x00C1);
  LCD_WriteReg(0xD632, 0x0003);
  LCD_WriteReg(0xD633, 0x00C1);

  /* LV2 Page 0 enable */
  LCD_WriteReg(0xF000, 0x0055);
  LCD_WriteReg(0xF001, 0x00AA);
  LCD_WriteReg(0xF002, 0x0052);
  LCD_WriteReg(0xF003, 0x0008);
  LCD_WriteReg(0xF004, 0x0000);

  /* 480x800 */
  LCD_WriteReg(0xB000, 0x0000);
  LCD_WriteReg(0xB001, 0x0005);
  LCD_WriteReg(0xB002, 0x0002);
  LCD_WriteReg(0xB003, 0x0005);
  LCD_WriteReg(0xB004, 0x0002);

  LCD_WriteReg(0xB600, 0x0008);
  LCD_WriteReg(0xB500, 0x0050);

  /* Source hold time */
  LCD_WriteReg(0xB600, 0x0005);

  /* Gate EQ control */
  LCD_WriteReg(0xB700, 0x0000);
  LCD_WriteReg(0xB701, 0x0000);

  /* Source EQ control (Mode 2) */
  LCD_WriteReg(0xB800, 0x0001);
  LCD_WriteReg(0xB801, 0x0005);
  LCD_WriteReg(0xB802, 0x0005);
  LCD_WriteReg(0xB803, 0x0005);

  /* Inversion mode (2-dot) */
  LCD_WriteReg(0xBC00, 0x0002);
  LCD_WriteReg(0xBC01, 0x0000);
  LCD_WriteReg(0xBC02, 0x0000);

  /* BOE setting */
  LCD_WriteReg(0xCC00, 0x0003);
  LCD_WriteReg(0xCC01, 0x0000);
  LCD_WriteReg(0xCC02, 0x0000);

  /* Frame rate */
  LCD_WriteReg(0xBD00, 0x0001);
  LCD_WriteReg(0xBD01, 0x0084);
  LCD_WriteReg(0xBD02, 0x0007);
  LCD_WriteReg(0xBD03, 0x0031);
  LCD_WriteReg(0xBD04, 0x0000);

  LCD_WriteReg(0xBA00, 0x0001);

  LCD_WriteReg(0xFF00, 0x00AA);
  LCD_WriteReg(0xFF01, 0x0055);
  LCD_WriteReg(0xFF02, 0x0025);
  LCD_WriteReg(0xFF03, 0x0001);

  /* Timing control 4H w/ 4-delay */
  // LCD_WriteReg(0x3600, 0x0040);
  LCD_WriteReg(0x3600, 0x0000);
  LCD_WriteReg(0x3500, 0x0000);

  /* RGB565 */
  LCD_WriteReg(0x3A00, 0x0005);

  /* Sleep out */
  LCD_WriteRegNo(0x1100);
  HAL_Delay(120);

  /* Display on */
  LCD_WriteRegNo(0x2900);
  HAL_Delay(10);

  /* Memory write */
  LCD_WriteRegNo(0x2C00);
}

void LCD_Init(void) {
  uint16_t id;

  LCD_Reset();
  HAL_Delay(50);

  id = LCD_ReadID();

  if (id == 0x5510) {
    LCD_NT35510_InitRegs();
  }

  LCD_BacklightOn();
}

void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
  /* Column address */
  LCD_WriteReg(0x2A00, (uint16_t)((x1 >> 8) & 0x00FFU));
  LCD_WriteReg(0x2A01, (uint16_t)(x1 & 0x00FFU));
  LCD_WriteReg(0x2A02, (uint16_t)((x2 >> 8) & 0x00FFU));
  LCD_WriteReg(0x2A03, (uint16_t)(x2 & 0x00FFU));

  /* Row address */
  LCD_WriteReg(0x2B00, (uint16_t)((y1 >> 8) & 0x00FFU));
  LCD_WriteReg(0x2B01, (uint16_t)(y1 & 0x00FFU));
  LCD_WriteReg(0x2B02, (uint16_t)((y2 >> 8) & 0x00FFU));
  LCD_WriteReg(0x2B03, (uint16_t)(y2 & 0x00FFU));

  /* Memory write */
  LCD_WriteRegNo(0x2C00);
}

void LCD_WriteColor(uint16_t color) { LCD_WriteData(color); }

void LCD_FillRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                  uint16_t color) {
  uint32_t total;
  uint32_t i;

  if (x1 > x2 || y1 > y2) {
    return;
  }

  if (x2 >= LCD_WIDTH) {
    x2 = LCD_WIDTH - 1U;
  }

  if (y2 >= LCD_HEIGHT) {
    y2 = LCD_HEIGHT - 1U;
  }

  total = (uint32_t)(x2 - x1 + 1U) * (uint32_t)(y2 - y1 + 1U);

  LCD_SetWindow(x1, y1, x2, y2);

  for (i = 0; i < total; i++) {
    LCD_WriteData(color);
  }
}

void LCD_Clear(uint16_t color) {
  LCD_FillRect(0, 0, LCD_WIDTH - 1U, LCD_HEIGHT - 1U, color);
}

void LCD_TestFillRed(void) {
  uint32_t i;

  /* 480 x 800 for NT35510 panel used by sample */
  LCD_WriteReg(0x2A00, 0x0000);
  LCD_WriteReg(0x2A01, 0x0000);
  LCD_WriteReg(0x2A02, 0x0001);
  LCD_WriteReg(0x2A03, 0x00DF);

  LCD_WriteReg(0x2B00, 0x0000);
  LCD_WriteReg(0x2B01, 0x0000);
  LCD_WriteReg(0x2B02, 0x0003);
  LCD_WriteReg(0x2B03, 0x001F);

  LCD_WriteRegNo(0x2C00);

  for (i = 0; i < 480UL * 800UL; i++) {
    LCD_WriteData(0xF800);
  }
}

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color) {
  if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
    return;
  }

  LCD_SetWindow(x, y, x, y);
  LCD_WriteColor(color);
}

void LCD_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
  int x = 0;
  int y = r;
  int d = 3 - 2 * r;

  while (x <= y) {
    LCD_DrawPoint(x0 + x, y0 + y, color);
    LCD_DrawPoint(x0 - x, y0 + y, color);
    LCD_DrawPoint(x0 + x, y0 - y, color);
    LCD_DrawPoint(x0 - x, y0 - y, color);

    LCD_DrawPoint(x0 + y, y0 + x, color);
    LCD_DrawPoint(x0 - y, y0 + x, color);
    LCD_DrawPoint(x0 + y, y0 - x, color);
    LCD_DrawPoint(x0 - y, y0 - x, color);

    if (d < 0) {
      d = d + 4 * x + 6;
    } else {
      d = d + 4 * (x - y) + 10;
      y--;
    }

    x++;
  }
}

void LCD_ShowRGB565Buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          const uint16_t *buf) {
  uint32_t i;
  uint32_t total;

  if (buf == NULL) {
    return;
  }

  if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
    return;
  }

  if ((x + w) > LCD_WIDTH) {
    w = LCD_WIDTH - x;
  }

  if ((y + h) > LCD_HEIGHT) {
    h = LCD_HEIGHT - y;
  }

  total = (uint32_t)w * h;

  LCD_SetWindow(x, y, x + w - 1, y + h - 1);

  for (i = 0; i < total; i++) {
    LCD_WriteColor(buf[i]);
  }
}