/*
 * led.c - f405 feather LED setup
 */

#ifndef __led__
#define __led__

#include "stm32f4xx_hal.h"

enum led_colors
{
	LED_BLACK,
	LED_BLUE,
	LED_RED,
	LED_MAGENTA,
	LED_GREEN,
	LED_CYAN,
	LED_YELLOW,
	LED_WHITE,
};

void LEDInit(void);
void LEDOn(void);
void LEDOff(void);
void LEDToggle(void);
void LEDSetColor(uint32_t color);
void LEDSetIdx(uint8_t idx);

#endif
