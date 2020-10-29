/*
 * ST7735.h - interface routines for ST7735 LCD.
 * shamelessly ganked from Adafruit_ST7735 library
 * 08-28-20 E. Brombaugh
 * 10-21-20 E. Brombaugh - updated for f405_codec_v2
 * 10-28-20 E. Brombaugh - updated for f405 feather + tftwing
 */


#ifndef __ST7735__
#define __ST7735__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

// dimensions for LCD on tiny TFT wing
#define ST7735_TFTWIDTH 80
#define ST7735_TFTHEIGHT 160

// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0  
#define ST7735_WHITE   0xFFFF

void ST7735_init(void);
void ST7735_setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void ST7735_fillScreen(uint16_t color);
void ST7735_drawPixel(int16_t x, int16_t y, uint16_t color);
void ST7735_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void ST7735_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void ST7735_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color);
void ST7735_fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
	uint16_t color);
uint16_t ST7735_Color565(uint8_t r, uint8_t g, uint8_t b);
void ST7735_bitblt(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *buf);
void ST7735_drawchar(int16_t x, int16_t y, uint8_t chr, 
	uint16_t fg, uint16_t bg);
void ST7735_drawstr(int16_t x, int16_t y, char *str,
	uint16_t fg, uint16_t bg);
void ST7735_setRotation(uint8_t m);
void ST7735_setVScroll(uint8_t s);

#ifdef __cplusplus
}
#endif

#endif
