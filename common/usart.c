/*
 * usart.c - usart diag support for f405 feather
 * 10-27-2020 E. Brombaugh
 */

#include "usart.h"

#define USART_TX_Pin GPIO_PIN_10
#define USART_TX_GPIO_Port GPIOB
#define USART_RX_Pin GPIO_PIN_11
#define USART_RX_GPIO_Port GPIOB

//USART_HandleTypeDef UsartHandle;
UART_HandleTypeDef huart3;

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Hang forever */
  while(1)
  {
  }
}

/* USART setup */
void setup_usart(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Setup USART GPIO */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.Pin = USART_TX_Pin;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(USART_TX_GPIO_Port, &GPIO_InitStructure);

	/* Configure USART Rx as alternate function push-pull */
	GPIO_InitStructure.Pin = USART_RX_Pin;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(USART_RX_GPIO_Port, &GPIO_InitStructure);

	/* Setup USART */
    __HAL_RCC_USART3_CLK_ENABLE();

	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart3) != HAL_OK)
	{
		Error_Handler();
	}
}

/*
 * output for tiny printf
 */
void usart_putc(void* p, char c)
{
	while(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET)
	{
	}
	USART3->DR = c;
}
