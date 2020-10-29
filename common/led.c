/*
 * led.c - f405 LED setup
 */

#include "led.h"
#include "cyclesleep.h"
#include "arm_math.h"

const uint32_t led_colors[8] =
{
	0x000000,	// black
	0x000001,	// blue
	0x000100,	// red
	0x000101,	// magenta
	0x010000,	// green
	0x010001,	// cyan
	0x010100,	// yellow
	0x010101,	// white
};



uint32_t T0H, T0L, T1H, T1L, TRST;

/*
 * Initialize the breakout board LED
 */
void LEDInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	float32_t clkfreq;
	
	/* Enable GPIO B Clock */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	/* Enable LD2 for output */
	GPIO_InitStructure.Pin =  GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStructure.Pull = GPIO_NOPULL ;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/* pre-compute timing */
	clkfreq = HAL_RCC_GetSysClockFreq();
	T0H = floorf(500e-9F * clkfreq + 0.5F)-20;
	T0L = floorf(2000e-9F * clkfreq + 0.5F)-20;
	T1H = floorf(1200e-9F * clkfreq + 0.5F)-20;
	T1L = floorf(1300e-9F * clkfreq + 0.5F)-20;
	TRST = floorf(50e-6F * clkfreq + 0.5F);
}

/*
 * Turn on Red LED
 */
void LEDOn(void)
{
	GPIOC->BSRR = GPIO_PIN_1;
}

/*
 * Turn off LED
 */
void LEDOff(void)
{
	GPIOC->BSRR = (GPIO_PIN_1<<16);
}

/*
 * Toggle LED
 */
void LEDToggle(void)
{
	GPIOC->ODR ^= GPIO_PIN_1;
}

/*
 * Set RGB value
 */
void LEDSetColor(uint32_t color)
{
	uint8_t i;
	
	/* loop over full 24-bit RGB word */
	__disable_irq();
	for(i=0;i<24;i++)
	{
		/* set output high */
		GPIOC->BSRR = 1;
		
		/* generate pulse for this bit */
		if(color & (1<<23))
		{
			cyclesleep(T1H);
			GPIOC->BSRR = (1<<16);
			cyclesleep(T1L);
		}
		else
		{
			cyclesleep(T0H);
			GPIOC->BSRR = (1<<16);
			cyclesleep(T0L);
		}
		
		/* next bit */
		color <<= 1;
	}
	__enable_irq();
	
	/* Built in delay */
	cyclesleep(TRST);
}

/*
 * set indexed color
 */
void LEDSetIdx(uint8_t idx)
{
	LEDSetColor(led_colors[idx&7]);
}
