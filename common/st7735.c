/*
 * ST7735.c - interface routines for ST7735 LCD.
 * shamelessly ganked from Adafruit_ST7735 library
 * 08-12-19 E. Brombaugh
 * 10-21-20 E. Brombaugh - updated for f405_codec_v2
 */

#include "st7735.h"
#include "font_8x8.h"
#include "shared_spi.h"
#include "tftwing.h"
#include "printf.h"

/* ----------------------- I/O definitions ----------------------- */
#define ST7735_CS_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#define ST7735_CS_GPIO_PORT GPIOC
#define ST7735_CS_PIN GPIO_PIN_7

#define ST7735_DC_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#define ST7735_DC_GPIO_PORT GPIOC
#define ST7735_DC_PIN GPIO_PIN_6

#define ST7735_CS_LOW()    (ST7735_CS_GPIO_PORT->BSRR=(ST7735_CS_PIN<<16))
#define ST7735_CS_HIGH()   (ST7735_CS_GPIO_PORT->BSRR=ST7735_CS_PIN)
#define ST7735_DC_CMD()    (ST7735_DC_GPIO_PORT->BSRR=(ST7735_DC_PIN<<16))
#define ST7735_DC_DATA()   (ST7735_DC_GPIO_PORT->BSRR=ST7735_DC_PIN)

#define ST_CMD            0x100
#define ST_CMD_DELAY      0x200
#define ST_CMD_END        0x400

#define ST77XX_NOP        0x00
#define ST77XX_SWRESET    0x01
#define ST77XX_RDDID      0x04
#define ST77XX_RDDST      0x09

#define ST77XX_SLPIN      0x10
#define ST77XX_SLPOUT     0x11
#define ST77XX_PTLON      0x12
#define ST77XX_NORON      0x13

#define ST77XX_INVOFF     0x20
#define ST77XX_INVON      0x21
#define ST77XX_DISPOFF    0x28
#define ST77XX_DISPON     0x29
#define ST77XX_CASET      0x2A
#define ST77XX_RASET      0x2B
#define ST77XX_RAMWR      0x2C
#define ST77XX_RAMRD      0x2E

#define ST77XX_PTLAR      0x30
#define ST77XX_COLMOD     0x3A
#define ST77XX_MADCTL     0x36

#define ST77XX_MADCTL_MY  0x80
#define ST77XX_MADCTL_MX  0x40
#define ST77XX_MADCTL_MV  0x20
#define ST77XX_MADCTL_ML  0x10
#define ST77XX_MADCTL_RGB 0x08
#define ST77XX_MADCTL_MH  0x04


#define ST77XX_RDID1      0xDA
#define ST77XX_RDID2      0xDB
#define ST77XX_RDID3      0xDC
#define ST77XX_RDID4      0xDD

#define ST7735_SCRLAR     0x33
#define ST7735_VSCSAD     0x37

#define ST7735_FRMCTR1    0xB1
#define ST7735_FRMCTR2    0xB2
#define ST7735_FRMCTR3    0xB3
#define ST7735_INVCTR     0xB4
#define ST7735_DISSET5    0xB6

#define ST7735_PWCTR1     0xC0
#define ST7735_PWCTR2     0xC1
#define ST7735_PWCTR3     0xC2
#define ST7735_PWCTR4     0xC3
#define ST7735_PWCTR5     0xC4
#define ST7735_VMCTR1     0xC5

#define ST7735_GMCTRP1    0xE0
#define ST7735_GMCTRN1    0xE1

#define ST7735_PWCTR6     0xFC

/* ----------------------- private variables ----------------------- */
// Initialization command sequence
const static uint16_t
  initlst[] = {
#if 0
    // ST7735B
    ST77XX_SWRESET | ST_CMD,        //  1: Software reset, no args, w/delay
    ST_CMD_DELAY | 50,              //  50 ms delay
    ST77XX_SLPOUT | ST_CMD ,        //  2: Out of sleep mode, no args, w/delay
	ST_CMD_DELAY | 500,             //  500 ms delay
    ST77XX_COLMOD | ST_CMD ,        //  3: Set color mode
      0x50,                         //     16-bit color
	ST_CMD_DELAY | 10,              //     10 ms delay
    ST7735_FRMCTR1 | ST_CMD ,       //  4: Frame rate control
      0x00,                         //     fastest refresh
      0x06,                         //     6 lines front porch
      0x03,                         //     3 lines back porch
	ST_CMD_DELAY | 10,              //     10 ms delay
    ST77XX_MADCTL | ST_CMD ,        //  5: Mem access ctrl (directions), 1 arg:
      0x08,                         //     Row/col addr, bottom-top refresh
    ST7735_DISSET5 | ST_CMD,        //  6: Display settings #5, 2 args:
      0x15,                         //     1 clk cycle nonoverlap, 2 cycle gate
                                    //     rise, 3 cycle osc equalize
      0x02,                         //     Fix on VTL
    ST7735_INVCTR | ST_CMD,         //  7: Display inversion control, 1 arg:
      0x0,                          //     Line inversion
    ST7735_PWCTR1 | ST_CMD,         //  8: Power control, 2 args + delay:
      0x02,                         //     GVDD = 4.7V
      0x70,                         //     1.0uA
    ST_CMD_DELAY | 10,              //     10 ms delay
    ST7735_PWCTR2 | ST_CMD,         //  9: Power control, 1 arg, no delay:
      0x05,                         //     VGH = 14.7V, VGL = -7.35V
    ST7735_PWCTR3 | ST_CMD,         // 10: Power control, 2 args, no delay:
      0x01,                         //     Opamp current small
      0x02,                         //     Boost frequency
    ST7735_VMCTR1 | ST_CMD,         // 11: Power control, 2 args + delay:
      0x3C,                         //     VCOMH = 4V
      0x38,                         //     VCOML = -1.1V
    ST_CMD_DELAY | 10,              //     10 ms delay
    ST7735_PWCTR6 | ST_CMD,         // 12: Power control, 2 args, no delay:
      0x11,
      0x15,
    ST7735_GMCTRP1 | ST_CMD,        // 13: Gamma Adjustments (pos. polarity), 16 args + delay:
      0x09, 0x16, 0x09, 0x20,       //     (Not entirely necessary, but provides
      0x21, 0x1B, 0x13, 0x19,       //      accurate colors)
      0x17, 0x15, 0x1E, 0x2B,
      0x04, 0x05, 0x02, 0x0E,
    ST7735_GMCTRN1 | ST_CMD,        // 14: Gamma Adjustments (neg. polarity), 16 args + delay:
      0x0B, 0x14, 0x08, 0x1E,       //     (Not entirely necessary, but provides
      0x22, 0x1D, 0x18, 0x1E,       //      accurate colors)
      0x1B, 0x1A, 0x24, 0x2B,
      0x06, 0x06, 0x02, 0x0F,
    ST_CMD_DELAY | 10,              //     10 ms delay
    ST77XX_CASET | ST_CMD,          // 15: Column addr set, 4 args, no delay:
      0x00, 0x02,                   //     XSTART = 2
      0x00, 0x81,                   //     XEND = 129
    ST77XX_RASET | ST_CMD,          // 16: Row addr set, 4 args, no delay:
      0x00, 0x02,                   //     XSTART = 1
      0x00, 0x81,                   //     XEND = 160
    ST77XX_NORON | ST_CMD,          // 17: Normal display on, no args, w/delay
    ST_CMD_DELAY | 10,              //     10 ms delay
    ST77XX_DISPON | ST_CMD,         // 18: Main screen turn on, no args, delay
    ST_CMD_DELAY | 500,             //     255 = max (500 ms) delay
	ST_CMD_END                      //  END OF LIST
#else
    // mini is ST7735R
    // 7735R init, part 1 (red or green tab)
                                    // 15 commands in list:
    ST77XX_SWRESET | ST_CMD,        //  1: Software reset, 0 args, w/delay
    ST_CMD_DELAY | 15,             //     150 ms delay
    ST77XX_SLPOUT | ST_CMD,         //  2: Out of sleep mode, 0 args, w/delay
    ST_CMD_DELAY | 50,             //     500 ms delay
    ST7735_FRMCTR1 | ST_CMD,        //  3: Framerate ctrl - normal mode, 3 arg:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2 | ST_CMD,        //  4: Framerate ctrl - idle mode, 3 args:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3 | ST_CMD,        //  5: Framerate - partial mode, 6 args:
      0x01, 0x2C, 0x2D,             //     Dot inversion mode
      0x01, 0x2C, 0x2D,             //     Line inversion mode
    ST7735_INVCTR | ST_CMD,         //  6: Display inversion ctrl, 1 arg:
      0x07,                         //     No inversion
    ST7735_PWCTR1 | ST_CMD,         //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                         //     -4.6V
      0x84,                         //     AUTO mode
    ST7735_PWCTR2 | ST_CMD,         //  8: Power control, 1 arg, no delay:
      0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    ST7735_PWCTR3 | ST_CMD,         //  9: Power control, 2 args, no delay:
      0x0A,                         //     Opamp current small
      0x00,                         //     Boost frequency
    ST7735_PWCTR4 | ST_CMD,         // 10: Power control, 2 args, no delay:
      0x8A,                         //     BCLK/2,
      0x2A,                         //     opamp current small & medium low
    ST7735_PWCTR5 | ST_CMD,         // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 | ST_CMD,         // 12: Power control, 1 arg, no delay:
      0x0E,
    ST77XX_INVOFF | ST_CMD,         // 13: Don't invert display, no args
    ST77XX_MADCTL | ST_CMD,         // 14: Mem access ctl (directions), 1 arg:
      0xC8,                         //     row/col addr, bottom-top refresh
    ST77XX_COLMOD | ST_CMD,         // 15: set color mode, 1 arg, no delay:
      0x05,                         //     16-bit color#endif

                                    // 7735R init, part 2 (mini 160x128)
                                    //  2 commands in list:
    ST77XX_CASET | ST_CMD,          //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F,                   //     XEND = 127
    ST77XX_RASET | ST_CMD,          //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F,                   //     XEND = 159

                                    // 7735R init, part 3 (red or green tab)
                                    //  4 commands in list:
    ST7735_GMCTRP1 | ST_CMD,        //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
      0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
      0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1 | ST_CMD,        //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
      0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
      0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST77XX_NORON | ST_CMD,          //  3: Normal display on, no args, w/delay
    ST_CMD_DELAY | 10,              //     10 ms delay
    ST77XX_DISPON | ST_CMD,         //  4: Main screen turn on, no args w/delay
    ST_CMD_DELAY | 10,             //     100 ms delay
	ST_CMD_END                      //  END OF LIST
#endif
};

/* SPI port handle */
SPI_HandleTypeDef SpiHandle;

/* LCD state */
uint8_t rowstart, colstart;
uint16_t _width, _height, rotation;

/* ----------------------- Private functions ----------------------- */
/*
 * Initialize SPI interface to LCD
 */
void ST7735_SPI_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable CS pin Clock */
	ST7735_CS_GPIO_CLK_ENABLE();

	/* Enable CS pin for output */
	GPIO_InitStructure.Pin =  ST7735_CS_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL ;
	HAL_GPIO_Init(ST7735_CS_GPIO_PORT, &GPIO_InitStructure);

	/* Enable DC pin Clock */
	ST7735_DC_GPIO_CLK_ENABLE();

	/* Enable DC pin for output */
	GPIO_InitStructure.Pin =  ST7735_DC_PIN;
	HAL_GPIO_Init(ST7735_DC_GPIO_PORT, &GPIO_InitStructure);

    /* init external SPI */
	Shared_SPI_Init();
}

/*
 * send single byte via SPI - cmd or data depends on bit 8
 */
void ST7735_write_byte(uint16_t dat)
{
	if((dat & ST_CMD) == ST_CMD)
		ST7735_DC_CMD();
	else
		ST7735_DC_DATA();

	ST7735_CS_LOW();

    Shared_SPI_WriteByte(dat&0xff);

	ST7735_CS_HIGH();
}

/* ----------------------- Public functions ----------------------- */
// Initialization for ST7735R red tab screens
void ST7735_init(void)
{
	// init the SPI port
	ST7735_SPI_Init();

	// default settings
	colstart = 24;
	rowstart = 0;
	_width  = ST7735_TFTWIDTH;
	_height = ST7735_TFTHEIGHT;
	rotation = 0;

	// Reset it
	tftwing_tftReset(0);
	HAL_Delay(10);
	tftwing_tftReset(1);
	HAL_Delay(10);

	// Send init command list
	uint16_t *addr = (uint16_t *)initlst, ms;
	while(*addr != ST_CMD_END)
	{
		if((*addr & ST_CMD_DELAY) != ST_CMD_DELAY)
			ST7735_write_byte(*addr++);
		else
		{
			ms = (*addr++)&0x1ff;        // strip delay time (ms)
			HAL_Delay(ms);
		}
	}

	// turn on the backlight
	tftwing_setBacklight(0);
}

// opens a window into display mem for bitblt
void ST7735_setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	uint8_t tx_buf[4];
	uint16_t sum;

    //__disable_irq();
    	ST7735_write_byte(ST77XX_CASET | ST_CMD); // Column addr set
	sum = x0+colstart;
	tx_buf[0] = sum>>8;
	tx_buf[1] = sum&0xff;
	sum = x1+colstart;
	tx_buf[2] = sum>>8;
	tx_buf[3] = sum&0xff;
	ST7735_DC_DATA();
	ST7735_CS_LOW();
	Shared_SPI_Blocking_PIO_WriteBytes(tx_buf, 4);
	ST7735_CS_HIGH();

	ST7735_write_byte(ST77XX_RASET | ST_CMD); // Row addr set
	sum = y0+rowstart;
	tx_buf[0] = sum>>8;
	tx_buf[1] = sum&0xff;
	sum = y1+rowstart;
	tx_buf[2] = sum>>8;
	tx_buf[3] = sum&0xff;
	ST7735_DC_DATA();
	ST7735_CS_LOW();
	Shared_SPI_Blocking_PIO_WriteBytes(tx_buf, 4);
	ST7735_CS_HIGH();

	ST7735_write_byte(ST77XX_RAMWR | ST_CMD); // write to RAM
    //__enable_irq();
}

// fill screen w/ single color
void ST7735_fillScreen(uint16_t color)
{
	ST7735_fillRect(0, 0, _width, _height, color);
}

// draw single pixel
void ST7735_drawPixel(int16_t x, int16_t y, uint16_t color)
{

	if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

	ST7735_setAddrWindow(x,y,x+1,y+1);

	ST7735_DC_DATA();
	ST7735_CS_LOW();

	Shared_SPI_WriteWord(__REVSH(color));

	ST7735_CS_HIGH();
}

// fast vert line
void ST7735_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((y+h-1) >= _height) h = _height-y;
	ST7735_setAddrWindow(x, y, x, y+h-1);

	ST7735_DC_DATA();
	ST7735_CS_LOW();

	Shared_SPI_Blocking_PIO_WriteWord(__REVSH(color), h);

	ST7735_CS_HIGH();
}

// fast horiz line
void ST7735_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((x+w-1) >= _width)  w = _width-x;
	ST7735_setAddrWindow(x, y, x+w-1, y);

	ST7735_DC_DATA();
	ST7735_CS_LOW();

	Shared_SPI_Blocking_PIO_WriteWord(__REVSH(color), w);

	ST7735_CS_HIGH();
}

/*
 *  draws a line
 */
void ST7735_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color)
{
    /* Bresenham algorithm */
    int16_t dx = x1 - x0 >= 0 ? x1 - x0 : x0 - x1;
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t dy = y1 - y0 <= 0 ? y1 - y0 : y0 - y1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;

    while((x0 != x1) && (y0 != y1)) {
        ST7735_drawPixel(x0, y0 , color);
        if (2 * err >= dy) {
            err += dy;
            x0 += sx;
        }
        if (2 * err <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}



// fill a rectangle
void ST7735_fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
	uint16_t color)
{
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;

	ST7735_setAddrWindow(x, y, x+w-1, y+h-1);

	/* prep tos end data */
	ST7735_DC_DATA();
	ST7735_CS_LOW();

	/* faster version keeps pipes full */
	Shared_SPI_Blocking_PIO_WriteWord(color, h*w);

	ST7735_CS_HIGH();
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t ST7735_Color565(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// bitblt a region to the display
void ST7735_bitblt(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *buf)
{
	ST7735_setAddrWindow(x, y, x+w-1, y+h-1);

	ST7735_DC_DATA();
	ST7735_CS_LOW();

#if 0
    /* start DMA */
    Shared_SPI_start_DMA_WriteBytes((uint8_t *)buf, 2*w*h);

    /* end DMA */
    Shared_SPI_end_DMA_WriteBytes();
#else
	/* PIO buffer send */
	Shared_SPI_Blocking_PIO_WriteBytes((uint8_t *)buf, 2*w*h);
#endif
	ST7735_CS_HIGH();
}

uint16_t gr_buff[64];

// Draw character direct to the display
void ST7735_drawchar(int16_t x, int16_t y, uint8_t chr,
	uint16_t fg, uint16_t bg)
{
    int16_t xt, yt;
	uint16_t i, j;
	uint8_t d;
	uint16_t *gptr = gr_buff;

    if((x>_width)||(y>_height))
        return;

    /* swap byte order of colors to ms-first for buffered writes */
    fg = __REVSH(fg);
    bg = __REVSH(bg);
    yt = y;

	/* convert font bitmap to colored glyph */
	for(i=0;i<8;i++)
	{
		d = fontdata[(chr<<3)+i];
        xt = x;
		for(j=0;j<8;j++)
		{
			if(d&0x80)
				*gptr++ = fg;
			else
				*gptr++ = bg;

			// next bit
			d <<= 1;

            /* update x counter and clip x */
            xt++;
            if(xt>(_width-1))
                break;
		}

        /* update y counter and clip y */
        yt++;
        if(yt>(_height-1))
            break;
	}

    /* compute actual dimensions */
    xt -= x;
    yt -= y;

    /* render to LCD */
	ST7735_bitblt(x, y, xt, yt, gr_buff);
}

// draw a string to the display
void ST7735_drawstr(int16_t x, int16_t y, char *str,
	uint16_t fg, uint16_t bg)
{
	uint8_t c;

	/* loop over string */
    //__disable_irq();
	while((c=*str++))
	{
		ST7735_drawchar(x, y, c, fg, bg);
		x+=8;
	}
    //__enable_irq();
}

// set orientation of display
void ST7735_setRotation(uint8_t m)
{
	ST7735_write_byte(ST77XX_MADCTL | ST_CMD);
	rotation = m % 4; // can't be higher than 3
	switch (rotation)
	{
		case 0:
			ST7735_write_byte(ST77XX_MADCTL_MX | ST77XX_MADCTL_MY );
			_width  = ST7735_TFTWIDTH;
			_height = ST7735_TFTHEIGHT;
			rowstart = 0;
			colstart = 24;
			break;

		case 1:
			ST7735_write_byte(ST77XX_MADCTL_MY | ST77XX_MADCTL_MV );
			_width  = ST7735_TFTHEIGHT;
			_height = ST7735_TFTWIDTH;
			rowstart = 24;
			colstart = 0;
			break;

		case 2:
			ST7735_write_byte(0);
			_width  = ST7735_TFTWIDTH;
			_height = ST7735_TFTHEIGHT;
			rowstart = 0;
			colstart = 24;
			break;

		case 3:
			ST7735_write_byte(ST77XX_MADCTL_MX | ST77XX_MADCTL_MV );
			_width  = ST7735_TFTHEIGHT;
			_height = ST7735_TFTWIDTH;
			rowstart = 24;
			colstart = 0;
			break;
	}
}

// set vertical scroll
void ST7735_setVScroll(uint8_t s)
{
	ST7735_write_byte(ST7735_VSCSAD | ST_CMD);
    ST7735_write_byte(0);
    ST7735_write_byte(s%ST7735_TFTHEIGHT);
}
