# RPI Oscilloscope

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




