/*
 * tftwing.c - I2C interface to Adafruit seesaw chip on TFT Wing
 * 10-27-2020 E. Brombaugh
 */

#include "tftwing.h"
#include "shared_i2c.h"
#include "printf.h"
#include "cyclesleep.h"

#define TFTWING_ADDR (0x5E<<1)
#define TFTWING_RESET_PIN 8
#define SEESAW_HW_ID_CODE 0x55 ///< seesaw HW ID code

/** Module Base Addreses
 *  The module base addresses for different seesaw modules.
 */
enum {
  SEESAW_STATUS_BASE = 0x00,
  SEESAW_GPIO_BASE = 0x01,
  SEESAW_SERCOM0_BASE = 0x02,

  SEESAW_TIMER_BASE = 0x08,
  SEESAW_ADC_BASE = 0x09,
  SEESAW_DAC_BASE = 0x0A,
  SEESAW_INTERRUPT_BASE = 0x0B,
  SEESAW_DAP_BASE = 0x0C,
  SEESAW_EEPROM_BASE = 0x0D,
  SEESAW_NEOPIXEL_BASE = 0x0E,
  SEESAW_TOUCH_BASE = 0x0F,
  SEESAW_KEYPAD_BASE = 0x10,
  SEESAW_ENCODER_BASE = 0x11,
};

/** GPIO module function addres registers
 */
enum {
  SEESAW_GPIO_DIRSET_BULK = 0x02,
  SEESAW_GPIO_DIRCLR_BULK = 0x03,
  SEESAW_GPIO_BULK = 0x04,
  SEESAW_GPIO_BULK_SET = 0x05,
  SEESAW_GPIO_BULK_CLR = 0x06,
  SEESAW_GPIO_BULK_TOGGLE = 0x07,
  SEESAW_GPIO_INTENSET = 0x08,
  SEESAW_GPIO_INTENCLR = 0x09,
  SEESAW_GPIO_INTFLAG = 0x0A,
  SEESAW_GPIO_PULLENSET = 0x0B,
  SEESAW_GPIO_PULLENCLR = 0x0C,
};

/** status module function addres registers
 */
enum {
  SEESAW_STATUS_HW_ID = 0x01,
  SEESAW_STATUS_VERSION = 0x02,
  SEESAW_STATUS_OPTIONS = 0x03,
  SEESAW_STATUS_TEMP = 0x04,
  SEESAW_STATUS_SWRST = 0x7F,
};

/** timer module function addres registers
 */
enum {
  SEESAW_TIMER_STATUS = 0x00,
  SEESAW_TIMER_PWM = 0x01,
  SEESAW_TIMER_FREQ = 0x02,
};

/** gpio modes
 */
enum {
  SEESAW_GPIO_MODE_OUTPUT = 0x00,
  SEESAW_GPIO_MODE_INPUT = 0x01,
  SEESAW_GPIO_MODE_INPUT_PULLUP = 0x02,
  SEESAW_GPIO_MODE_INPUT_PULLDOWN = 0x03,
};

/*
 * error handler for TFT
 */
void tftwing_i2c_error(uint8_t code)
{
	printf("tftwing I2C error code %d - resetting\n\r", code);
	shared_i2c_reset();
}

/*
 * read a buffer from seesaw
 */
uint32_t seesaw_readbuf(uint8_t reghi, uint8_t reglo, uint8_t *buf, uint8_t sz)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t i2c_msg[2];
	
	/* build message */
	i2c_msg[0] = reghi;
	i2c_msg[1] = reglo;

	/* send reg addr */
	status = HAL_I2C_Master_Transmit(&hi2c1, TFTWING_ADDR, i2c_msg, 2, 100);

	/* Check the communication status */
	if(status != HAL_OK)
	{
		tftwing_i2c_error(1);
		return status;
	}
	
	/* delay 100us */
	cyclesleep(16800);
	
	/* receive data */
	status = HAL_I2C_Master_Receive(&hi2c1, TFTWING_ADDR, buf, sz, 100);

	/* Check the communication status */
	if(status != HAL_OK)
	{
		tftwing_i2c_error(2);
	}

	return status;
}

/*
 * write a buffer to seesaw
 */
uint32_t seesaw_writebuf(uint8_t reghi, uint8_t reglo, uint8_t *buf, uint8_t sz)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t i, i2c_msg[18];
	
	if(sz>16)
		return HAL_ERROR;
	
	/* build message */
	i2c_msg[0] = reghi;
	i2c_msg[1] = reglo;
	for(i=0;i<sz;i++)
		i2c_msg[i+2]=buf[i];
	
	/* send reg addr */
	status = HAL_I2C_Master_Transmit(&hi2c1, TFTWING_ADDR, i2c_msg, 2+sz, 100);

	/* Check the communication status */
	if(status != HAL_OK)
	{
		tftwing_i2c_error(3);
	}

	return status;
}

/*
 * software reset to seesaw
 */
uint32_t seesaw_swrst(void)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t i2c_msg[3];
	
	/* build message */
	i2c_msg[0] = SEESAW_STATUS_BASE;
	i2c_msg[1] = SEESAW_STATUS_SWRST;
	i2c_msg[2] = 0xFF;
	
	/* send reg addr */
	status = HAL_I2C_Master_Transmit(&hi2c1, TFTWING_ADDR, i2c_msg, 3, 100);

	/* Check the communication status */
	if(status != HAL_OK)
	{
		tftwing_i2c_error(4);
	}
	
	HAL_Delay(500);

	return status;
}

/*
 * set seesaw gpio multiple pin mode
 */
void seesaw_pinModeBulk(uint32_t pins, uint8_t mode)
{
	uint8_t cmd[] = {(uint8_t)(pins >> 24), (uint8_t)(pins >> 16),
		(uint8_t)(pins >> 8), (uint8_t)pins};
	switch (mode)
	{
		case SEESAW_GPIO_MODE_OUTPUT:
			seesaw_writebuf(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRSET_BULK, cmd, 4);
			break;
		case SEESAW_GPIO_MODE_INPUT:
			seesaw_writebuf(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, cmd, 4);
			break;
		case SEESAW_GPIO_MODE_INPUT_PULLUP:
			seesaw_writebuf(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, cmd, 4);
			seesaw_writebuf(SEESAW_GPIO_BASE, SEESAW_GPIO_PULLENSET, cmd, 4);
			seesaw_writebuf(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_SET, cmd, 4);
			break;
		case SEESAW_GPIO_MODE_INPUT_PULLDOWN:
			seesaw_writebuf(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, cmd, 4);
			seesaw_writebuf(SEESAW_GPIO_BASE, SEESAW_GPIO_PULLENSET, cmd, 4);
			seesaw_writebuf(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_CLR, cmd, 4);
			break;
	}
}

/*
 * read multiple GPIO pins
 */
uint32_t seesaw_digitalReadBulk(uint32_t pins)
{
	uint32_t ret;
	uint8_t buf[4];
	
	seesaw_readbuf(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK, buf, 4);
	ret = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
		((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
	return ret & pins;
}

/*
 * write multiple GPIO pins
 */
void seesaw_digitalWriteBulk(uint32_t pins, uint8_t value)
{
	uint8_t cmd[4];
	
	cmd[0] = (uint8_t)(pins >> 24);
	cmd[1] = (uint8_t)(pins >> 16);
	cmd[2] = (uint8_t)(pins >> 8);
	cmd[3] = (uint8_t)pins;
				   
	if (value)
		seesaw_writebuf(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_SET, cmd, 4);
	else
		seesaw_writebuf(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_CLR, cmd, 4);
}

/*
 * start up interface to tftwing
 * for some reason this fails if it's the first I2C thing done
 * so do something else first (init other periphs on bus)
 */
uint8_t tftwing_init(void)
{
	uint8_t id = 0;
	
	/* dummy write seems to help seesaw wake up */
	HAL_I2C_Master_Transmit(&hi2c1, 0x10, &id, 1, 100);

	/* reset the seesaw */
	if(seesaw_swrst())
	{
		printf("tftwing_init: SWRst failed\n\r");
		return 1;
	}
	
	/* get the seesaw ID */
	if(seesaw_readbuf(SEESAW_STATUS_BASE, SEESAW_STATUS_HW_ID, &id, 1))
	{
		printf("tftwing_init: ID read failed\n\r");
		return 1;
	}
	
	/* check for correct ID */
	if(id != SEESAW_HW_ID_CODE)
	{
		printf("tftwing_init: ID code mismatch\n\r");
		return 1;
	}
	
	seesaw_pinModeBulk((1<<TFTWING_RESET_PIN), SEESAW_GPIO_MODE_OUTPUT);
	seesaw_pinModeBulk(TFTWING_BUTTON_ALL, SEESAW_GPIO_MODE_INPUT_PULLUP);

	return 0;
}

/*
 * set LCD backlight PWM value
 */
void tftwing_setBacklight(uint16_t value)
{
	uint8_t cmd[] = {0x00, (uint8_t)(value >> 8), (uint8_t)value};
	seesaw_writebuf(SEESAW_TIMER_BASE, SEESAW_TIMER_PWM, cmd, 3);
}

/*
 * set LCD backlight PWM Freq
 */
void tftwing_setBacklightFreq(uint16_t freq)
{
	uint8_t cmd[] = {0x0, (uint8_t)(freq >> 8), (uint8_t)freq};
	seesaw_writebuf(SEESAW_TIMER_BASE, SEESAW_TIMER_FREQ, cmd, 3);
}

/*
 * control LCD reset pin
 */
void tftwing_tftReset(uint8_t rst)
{
	seesaw_digitalWriteBulk(1ul << TFTWING_RESET_PIN, rst);
}

/*
 * get status of 7 buttons on tftwing
 */
uint32_t tftwing_readButtons(void)
{
	return seesaw_digitalReadBulk(TFTWING_BUTTON_ALL);
}

