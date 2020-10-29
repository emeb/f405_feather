/*
 * systick.c - f405 codec v2 systick setup
 */

#include "systick.h"
#include "debounce.h"

/*
 * Encoder Connections
 *
 * ENC_A  PB0
 * ENC_B  PB1
 * ENC_E  PB2
 *
 */

#define ENC_A_PORT GPIOB
#define ENC_A_PIN GPIO_PIN_0
#define ENC_B_PORT GPIOB
#define ENC_B_PIN GPIO_PIN_1
#define ENC_E_PORT GPIOB
#define ENC_E_PIN GPIO_PIN_2

int16_t enc_val;
debounce_state btn_dbs[NUM_DBS];
uint8_t btn_fe[NUM_DBS], btn_re[NUM_DBS];
uint8_t emute_state, emute_event, emute_holdoff;

const GPIO_TypeDef *btn_ports[NUM_DBS] =
{
	ENC_A_PORT,
	ENC_B_PORT,
	ENC_E_PORT,
};

const uint16_t btn_pins[NUM_DBS] =
{
	ENC_A_PIN,
	ENC_B_PIN,
	ENC_E_PIN,
};

const uint8_t btn_db_lens[NUM_DBS] =
{
	2,
	2,
	15,
};

const GPIO_PinState btn_state_act[NUM_DBS] =
{
	GPIO_PIN_SET,	// Encoder phases active high
	GPIO_PIN_SET,
	GPIO_PIN_RESET,
};

/*
 * Initialize the encoder
 */
void systick_init(void)
{
	uint8_t i;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO A Clock */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* Enable buttons for output */
	GPIO_InitStructure.Pin =  ENC_A_PIN | ENC_B_PIN | ENC_E_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStructure.Pull = GPIO_PULLUP ;
	HAL_GPIO_Init(ENC_A_PORT, &GPIO_InitStructure);

	/* set up debounce objects for buttons & encoder */
	for(i=0;i<NUM_DBS;i++)
	{
		init_debounce(&btn_dbs[i], btn_db_lens[i]);
		btn_fe[i] = 0;
		btn_re[i] = 0;
	}
    enc_val = 0;
}

/*
 * check status of buttons
 */
uint8_t systick_get_button(uint8_t btn)
{
	return btn_dbs[btn].state;
}

/*
 * check for falling edge of buttons
 */
uint8_t systick_button_fe(uint8_t btn)
{
	uint8_t result = btn_fe[btn];
	btn_fe[btn] = 0;
	return result;
}

/*
 * check for rising edge of buttons
 */
uint8_t systick_button_re(uint8_t btn)
{
	uint8_t result = btn_re[btn];
	btn_re[btn] = 0;
	return result;
}

/*
 * get state of encoder shaft
 */
int16_t systick_get_encoder(void)
{
	int16_t result = enc_val;
	enc_val = 0;
	return result;
}

/*
 * needed by HAL
 */
void SysTick_Handler(void)
{
	uint8_t i;

	/* Needed by HAL! */
	HAL_IncTick();

	/* gather inputs and run all debouncers */
	for(i=0;i<NUM_DBS;i++)
	{
		debounce(&btn_dbs[i], (HAL_GPIO_ReadPin((GPIO_TypeDef *)btn_ports[i],
			btn_pins[i]) != btn_state_act[i]));
		btn_fe[i] |= btn_dbs[i].fe;
		btn_re[i] |= btn_dbs[i].re;
	}

	/* decode encoder phases - only one edge allowed per */
	if(btn_dbs[ENC_A].re)
	{
        if(btn_dbs[ENC_B].state == 0)
            enc_val++;
        else
            enc_val--;
    }
}
