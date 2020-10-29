/*
 * shared_i2c.c - shared I2C bus basic routines
 * 07-12-19 E. Brombaugh
 */

#include "shared_i2c.h"

I2C_HandleTypeDef hi2c1;

/*
 * initialize shared I2C bus
 */
void shared_i2c_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C2_SCL
    PB7     ------> I2C2_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */

	/* reset the I2C bus */
	shared_i2c_reset();
}

void shared_i2c_reset(void)
{
	/* Enable the I2C1 peripheral clock & reset it */
    __HAL_RCC_I2C1_CLK_ENABLE();
	__HAL_RCC_I2C1_FORCE_RESET();
	__HAL_RCC_I2C1_RELEASE_RESET();

	/* De-initialize the I2C communication bus */
	HAL_I2C_DeInit(&hi2c1);

	/* I2C1 peripheral configuration */
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	/* Init the I2C */
	HAL_I2C_Init(&hi2c1);
}
