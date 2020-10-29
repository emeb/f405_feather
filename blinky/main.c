/*
 * main.c - part of f405_feather blinky
 */

#include "stm32f4xx_hal.h"
#include "cyclesleep.h"
#include "led.h"
#include "usart.h"
#include "printf.h"
#include "shared_i2c.h"
#include "oled.h"
#include "tftwing.h"
#include "st7735.h"
#include "adc.h"
#include "arm_math.h"

/* uncomment this to enable the OLED */
//#define OLED

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/*
 * main routine
 */
int main(void)
{
	uint8_t cnt = 0, i;
	char txtbuf[32];
	
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();
	
	/* init the UART for diagnostics */
	setup_usart();
	init_printf(0,usart_putc);
	printf("\n\n\rF405 Feather Blink\n\r");
	printf("\n");
	printf("SYSCLK = %d\n\r", HAL_RCC_GetSysClockFreq());
	printf("\n");
	
	/* start cycle timer */
	cyccnt_enable();
	printf("Cycle Counter initialized\n\r");

	/* initialize LED */
	LEDInit();
	printf("LED initialized\n\r");
	
	/* I2C */
	shared_i2c_init();
	printf("I2C initialized\n\r");
	
#ifdef OLED
	/* OLED */
	printf("OLED initialized - result = %d\n\r", oled_init());
	
    /* test circle draw */
    oled_Circle(0, 64, 16, 15, 1);
    oled_refresh(0);	
#endif

	/* TFTWing seesaw */
	printf("TFTWing seesaw initialized - result = %d\n\r", tftwing_init());
	
	/* TFTWing LCD */
	ST7735_init();
	ST7735_fillScreen(ST7735_BLACK);
	ST7735_setRotation(3);
	ST7735_drawstr(0, 0, "Hello, World!", ST7735_GREEN, ST7735_BLACK);
	printf("TFTWing LCD initialized\n\r");
	
	/* ADC */
	printf("ADC initialized - result = %d\n\r", ADC_Init());
	
    /* Infinite loop */
    while(1)
    {
		/* update color */
		LEDSetIdx(cnt>>4);
		
		/* blink red */
		LEDToggle();
		
#ifdef OLED
		/* sweep radar */
		float32_t phs = 6.2832F*(float32_t)cnt/256.0F;
		oled_line(0, 64, 16, 64+floorf(13.0F*cosf(phs)),
			16-floorf(13.0F*sinf(phs)), 0);
		cnt++;
		phs = 6.2832F*(float32_t)cnt/256.0F;
		oled_line(0, 64, 16, 64+floorf(13.0F*cosf(phs)),
			16-floorf(13.0F*sinf(phs)), 1);
		oled_refresh(0);
#else
		cnt++;
#endif
		
		/* update buttons */
		printf("tftwing buttons = 0x%02X\r", tftwing_readButtons());
		
		/* update ADC readings */
		for(i=0;i<5;i++)
		{
			sprintf(txtbuf, "%1d:%4d", i, ADC_GetChl(i));
			ST7735_drawstr(0, 8*(i+1), txtbuf, ST7735_YELLOW, ST7735_BLACK);
		}
		
		/* delay */
		HAL_Delay(10);
    }
}

/*
 * needed by HAL
 */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

