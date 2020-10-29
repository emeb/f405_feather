/*
 * usart.h - usart diag support for f405 feather
 * 10-27-2020 E. Brombaugh
 */

#ifndef __usart__
#define __usart__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

void setup_usart(void);
void usart_putc(void* p, char c);

#ifdef __cplusplus
}
#endif

#endif
