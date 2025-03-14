# RPI Oscilloscope
Contributors: Irene Geng, Tony Liu, Yuchi Hsu, Andreas Alexandrou

We provide an end-to-end oscilloscope implementation, complete with user controls, and measurement capabilities. We use the following hardware:
2 RPIs
1 ADS1115 Analogue to Digital Converter
3 SD11306 Displays
Our custom signal generator

Our oscilloscope features three knob controls, allowing the user to offset the displayed signal in the Y dimension, and change the horizontal, and vertical scaling. Additionally, we use a second Raspberry Pi to perform further calculations on the sampled signal and provide peak-to-peak amplitude, and frequency measurements.

Our ADC module features a valid pin, which is triggered when the measurement settles and is available to read. We configure a GPIO interrupt to observe this pin, and perform an ADC read accordingly. Upon collecting a sufficient number of samples, we flush our buffer to the display.

Computing the peak-to-peak amplitude is trivial, but frequency measurement requires a Fast Fourier Transform (FFT), which is computationally expensive. To offset this cost, we send full data buffers to a second RPI over NRF. The second PI computes the FFT and sends back the calculated measurements. We configure a second interrupt handler to trigger whenever we receive the packet and update the frequency measurement on the display.

To accommodate the showcase, we create a custom signal generator, which can be configured to output sinusoids from 47 up to 106 Hz.

We developed the following:
I2C driver
ADS1115 ADC driver
SSD1306 display driver
Graphics library
A performant FFT implementation
Interrupt-driven oscilloscope code
NRF24L01 packet fragmenting

## Setup

### Setup pi-install
Go to `/bin`, copy the `pi-install.macos` or `pi-install.linux`(depends on your OS) and rename it to `pi-install`.

### Add CS140E_FINAL_PROJ_PATH to your environment
In your `.zshrc` or `.bashrc`, add the following:
```
export CS140E_FINAL_PROJ_PATH=<path to this repo>
```

## Git

### Before you commit
Run `make clean` in the root directory to clean all the files.

## Makefile

When you create a new folder in `src`, you need to add it to the Makefile.

In the `src/Makefile`, add the following:
```
clean:
    ...existing clean rules...
	make clean -C <name of the new folder>
```

Also, create a Makefile in the new folder with clean rules.

## I2C

For now, we will first use `staff-faster-i2c.o` to speed up development. We will implement our own `i2c.c` later.

### Usage
I have configured the Makefile in `libpi`, so you can directly use I2C functions like this:
```c
#include "i2c.h"

void notmain(void) {
    i2c_init();
    // Initialize the OLED display
    uint8_t initSequence[26] = {0x00,0xAE,0xA8,0x3F,0xD3,0x00,0x40,0xA1,0xC8,0xDA,0x12,0x81,0x7F,
                                          0xA4,0xA6,0xD5,0x80,0x8D,0x14,0xD9,0x22,0xD8,0x30,0x20,0x00,0xAF};
    i2c_write(0x3C, initSequence, 26);
    ...
}
```
No need to setup I2C in your local Makefile.

### I2C API
Read `libpi/include/i2c.h` for more details.





