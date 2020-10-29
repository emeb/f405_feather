# Makefile for STM32F405 w/ HAL
# 10-20-2020 E. Brombaugh

# sub directories
VPATH = .:../CMSIS:../HAL:../common

# Object files
OBJECTS =   startup_stm32f405xx.o system_stm32f4xx.o main.o printf.o \
			usart.o cyclesleep.o led.o shared_i2c.o oled.o adc.o \
			arial_24_bold_32_numeral.o tftwing.o shared_spi.o st7735.o \
            stm32f4xx_hal_gpio.o stm32f4xx_hal_rcc.o stm32f4xx_hal_cortex.o \
			stm32f4xx_hal.o stm32f4xx_hal_pwr_ex.o stm32f4xx_hal_uart.o \
            stm32f4xx_hal_rcc_ex.o stm32f4xx_hal_i2c.o stm32f4xx_hal_spi.o \
			stm32f4xx_hal_adc.o stm32f4xx_hal_dma.o 
			
# Linker script
LDSCRIPT = STM32F405RGTx_FLASH.ld

# Compiler Flags
CFLAGS  = -g -O3 -ffunction-sections -std=gnu99 -Wall -flto
CFLAGS += -I. -I../CMSIS -I../HAL -I../common
CFLAGS += -DARM_MATH_CM4 -DUSE_HAL_DRIVER
CFLAGS += -DSTM32F405xx -D'HSE_VALUE=((uint32_t)12000000)'
CFLAGS += -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -mlittle-endian -mthumb
AFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m4
LFLAGS  = $(CFLAGS) -nostartfiles -T $(LDSCRIPT) -Wl,-Map=main.map
LFLAGS += -Wl,--gc-sections -Wl,--print-memory-usage
LFLAGS += --specs=nano.specs
CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

# Executables
#ARCH = arm-none-eabi
ARCH = /opt/launchpad/gcc-arm-none-eabi-7-2018-q2-update/bin/arm-none-eabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld -v
AS = $(ARCH)-as
OBJCPY = $(ARCH)-objcopy
OBJDMP = $(ARCH)-objdump
GDB = $(ARCH)-gdb
OPENOCD = openocd

CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

# Targets
all: main.bin

clean:
	-rm -f $(OBJECTS) crt.lst *.lst *.elf *.bin *.map *.dmp

#flash: gdb_flash
flash: oocd_flash

oocd_flash: main.elf
	$(OPENOCD) -f openocd_stlink.cfg -c "program main.elf verify reset exit"

gdb_flash: main.elf
	$(GDB) -x flash_cmd.gdb -batch
	stty sane

disassemble: main.elf
	$(OBJDMP) -d main.elf > main.dis
	
dist:
	tar -c *.h *.c *.s Makefile *.cmd *.cfg openocd_doflash | gzip > minimal_hello_world.tar.gz

main.ihex: main.elf
	$(OBJCPY) --output-target=ihex main.elf main.ihex

main.bin: main.elf 
	$(OBJCPY) $(CPFLAGS) main.elf main.bin
	$(OBJDMP) $(ODFLAGS) main.elf > main.dmp
	ls -l main.elf main.bin

main.elf: $(OBJECTS) $(LDSCRIPT)
	$(CC) $(LFLAGS) -o main.elf $(OBJECTS) -lnosys -lm

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

