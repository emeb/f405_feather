/*
 * tftwing.h - I2C interface to Adafruit seesaw chip on TFT Wing
 * 10-27-2020 E. Brombaugh
 */

#ifndef __tftwing__
#define __tftwing__

#include "stm32f4xx.h"

#define TFTWING_BACKLIGHT_ON 0       // inverted output!
#define TFTWING_BACKLIGHT_OFF 0xFFFF // inverted output!

#define TFTWING_BUTTON_UP_PIN 2
#define TFTWING_BUTTON_UP (1UL << TFTWING_BUTTON_UP_PIN)

#define TFTWING_BUTTON_DOWN_PIN 4
#define TFTWING_BUTTON_DOWN (1UL << TFTWING_BUTTON_DOWN_PIN)

#define TFTWING_BUTTON_LEFT_PIN 3
#define TFTWING_BUTTON_LEFT (1UL << TFTWING_BUTTON_LEFT_PIN)

#define TFTWING_BUTTON_RIGHT_PIN 7
#define TFTWING_BUTTON_RIGHT (1UL << TFTWING_BUTTON_RIGHT_PIN)

#define TFTWING_BUTTON_SELECT_PIN 11
#define TFTWING_BUTTON_SELECT (1UL << TFTWING_BUTTON_SELECT_PIN)

#define TFTWING_BUTTON_A_PIN 10
#define TFTWING_BUTTON_A (1UL << TFTWING_BUTTON_A_PIN)

#define TFTWING_BUTTON_B_PIN 9
#define TFTWING_BUTTON_B (1UL << TFTWING_BUTTON_B_PIN)

#define TFTWING_BUTTON_ALL                                                     \
  (TFTWING_BUTTON_UP | TFTWING_BUTTON_DOWN | TFTWING_BUTTON_LEFT |             \
   TFTWING_BUTTON_RIGHT | TFTWING_BUTTON_SELECT | TFTWING_BUTTON_A |           \
   TFTWING_BUTTON_B)

uint8_t tftwing_init(void);
void tftwing_setBacklight(uint16_t value);
void tftwing_setBacklightFreq(uint16_t freq);
void tftwing_tftReset(uint8_t rst);
uint32_t tftwing_readButtons(void);

#endif
