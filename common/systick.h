/*
 * systick.h - f405 codec v2 systick setup
 */

#ifndef __systick__
#define __systick__

#include "stm32f4xx_hal.h"

#define NUM_DBS 3

/* array indexes for debouncers */
enum buttons
{
	ENC_A,
	ENC_B,
	ENC_E,
};

void systick_init(void);
uint8_t systick_get_button(uint8_t btn);
uint8_t systick_button_fe(uint8_t btn);
uint8_t systick_button_re(uint8_t btn);
int16_t systick_get_encoder(void);

#endif
