/*
 * adc.c - adc setup
 */

#include "adc.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

uint16_t adc_rawbuf[ADC_NUMCHLS], adc_procbuf[ADC_NUMCHLS];

#define OS_RATIO 32
#define OS_SHIFT 5
int32_t adc_acc[ADC_NUMCHLS];
uint16_t adc_dly[ADC_NUMCHLS][OS_RATIO];
uint16_t adc_dly_ptr;

/* Diagnostic flag */
//#define ENABLE_ADCDIAG
#ifdef ENABLE_ADCDIAG
#define FLAG_0  GPIOC->BSRR=(1<<(14+16))
#define FLAG_1  GPIOC->BSRR=(1<<14)
#else
#define FLAG_0
#define FLAG_1
#endif

/*
 * init the ADC
 */
uint8_t ADC_Init(void)
{
    uint8_t result = 0;
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    ADC_ChannelConfTypeDef sConfig = {0};
    DMA_Stream_TypeDef *stream_adc1;
    uint16_t i, j;

    /* init buffers */
	for(i=0;i<ADC_NUMCHLS;i++)
	{
        adc_rawbuf[i] = 0;
        adc_procbuf[i] = 0;
		adc_acc[i] = 0;
		for(j=0;j<OS_RATIO;j++)
			adc_dly[i][j] = 0;
	}
	adc_dly_ptr = 0;

#ifdef ENABLE_ADCDIAG
    __HAL_RCC_GPIOC_CLK_ENABLE();

	/* Enable PC14 for diagnostic output */
	GPIO_InitStruct.Pin =  GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Pull = GPIO_NOPULL ;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
#endif

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA3     ------> ADC1_IN3 V_DIV
    PA6     ------> ADC1_IN6 A2
    PA7     ------> ADC1_IN7 A3
    PC4     ------> ADC1_IN14 A4
    PC5     ------> ADC1_IN15 A5
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = ENABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = ADC_NUMCHLS;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        result += 1;
    }

    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
    */
    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank = 1;
    //sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    //sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
    //sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        result += 2;
    }
    sConfig.Channel = ADC_CHANNEL_6;
    sConfig.Rank = 2;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        result += 4;
    }
    sConfig.Channel = ADC_CHANNEL_7;
    sConfig.Rank = 3;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        result += 8;
    }
    sConfig.Channel = ADC_CHANNEL_14;
    sConfig.Rank = 4;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        result += 16;
    }
    sConfig.Channel = ADC_CHANNEL_15;
    sConfig.Rank = 5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        result += 16;
    }

    /* ADC1 DMA Init */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* ADC1 Init */
    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
        result += 32;
    }

    __HAL_LINKDMA(&hadc1,DMA_Handle,hdma_adc1);

    /* set up DMA details */
	stream_adc1 = hdma_adc1.Instance;
	stream_adc1->CR &= (uint32_t)(~DMA_SxCR_DBM);
	stream_adc1->NDTR = (uint32_t)ADC_NUMCHLS;
	stream_adc1->PAR = (uint32_t)&hadc1.Instance->DR;
	stream_adc1->M0AR = (uint32_t)&adc_rawbuf;

	/* Enable the DMA transfer complete interrupt */
	__HAL_DMA_ENABLE_IT(&hdma_adc1, DMA_IT_TC);

	/* DMA2_Stream0_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    /* Enable ADC DMA requests */
    SET_BIT(hadc1.Instance->CR2, ADC_CR2_DMA);

	/* Enable DMA */
	__HAL_DMA_ENABLE(&hdma_adc1);

	/* enable ADC */
    __HAL_ADC_ENABLE(&hadc1);
    hadc1.Instance->CR2 |= ADC_CR2_SWSTART;

    return result;
}

/*
 * get ADC data from processed buffer
 */
uint16_t ADC_GetChl(uint8_t chl)
{
    return adc_procbuf[chl];
}

void DMA2_Stream0_IRQHandler(void)
{
    uint8_t i;

	FLAG_1;

	/* Transfer complete interrupt */
	if (__HAL_DMA_GET_FLAG(&hdma_adc1, DMA_FLAG_TCIF0_4) != RESET)
	{
		/* Clear the Interrupt flag */
		__HAL_DMA_CLEAR_FLAG(&hdma_adc1, DMA_FLAG_TCIF0_4);

		/* filter raw buffer to holding buffer */
		for(i=0;i<ADC_NUMCHLS;i++)
		{
			/* add new, subtract old to acc */
			adc_acc[i] -= adc_dly[i][adc_dly_ptr];
			adc_acc[i] += adc_rawbuf[i];

			/* store new in buffer */
			adc_dly[i][adc_dly_ptr] = adc_rawbuf[i];

			/* normalize acc to output */
			adc_procbuf[i] = adc_acc[i]>>OS_SHIFT;
        }

        adc_dly_ptr = (adc_dly_ptr+1)%OS_RATIO;
    }

    FLAG_0;
}
