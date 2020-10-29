# f405_feather
Bare-metal projects for the Adafruit STM32F405 Feather board

## Introduction

I used to do a lot of work with the STM32F405RGT6 MCU - a few open-source
projects and even a few commercial projects. It was a decent machine when it
first came out, being one of the first reasonably priced ARM Cortex M4F machines
out there. Now it's roughly ten years later and it shows its age a bit -
some of the on-chip peripherals are a bit wonky and there are many more newer
parts available with higher clock speeds, more memory and better peripherals.
Nonetheless, I was pleasantly surprised to see that Adafruit had a cute little
"Feather" format breakout board for this processor that brings out most of the
I/O and has a lot of nice extras so I bought one a few months ago.

Adafruit always supports their hardware well but their platforms of choice are
not mine - they lean towards the Arduino IDE and more recently CircuitPython.
Those are both great places for beginners to start or to lash something
together quickly, but they do leave some performance on the table. I prefer
to use bare metal C code in a "Big Loop" style. I use the ST HAL library to
get peripherals up and running with minimal fuss, but then I tend to talk
directly to registers once things are going. Hence this code repo which contains
basic drivers for many of the features on the F405 Feather, as well as some
extra stuff to control the Tiny TFT Wing board that plugs into it.

## Usage

The primary application is in the "blinky" directory. There's a basic Makefile
for building the binary and you'll want a recent install of the ARM GCC compiler,
along with Make and OpenOCD to complete the loading. You may need to modify
the make variables for your specific paths, etc.

