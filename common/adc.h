/*
 * adc.h - adc setup
 */

#ifndef __adc__
#define __adc__

#include "stm32f4xx_hal.h"

/* Number of ADC channels we're scanning */
#define ADC_NUMCHLS 5

uint8_t ADC_Init(void);
uint16_t ADC_GetChl(uint8_t chl);

#endif
