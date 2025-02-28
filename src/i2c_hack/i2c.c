#include "rpi.h"

#define BSC1_BASE   0x20804000
#define BSC_C       (BSC1_BASE + 0x0)   // Control register
#define BSC_S       (BSC1_BASE + 0x4)   // Status register
#define BSC_DIV     (BSC1_BASE + 0x14)  // Clock divider register


void my_i2c_init(void) {
    dev_barrier();
    // Set GPIO pins 2 and 3 for I2C function (alt function 0)
    gpio_set_function(2, 4);  // SDA
    gpio_set_function(3, 4);  // SCL
    dev_barrier();

    // Reset BSC clock, controller, and status
    PUT32(BSC_DIV, 0x5dc);
    PUT32(BSC_C, 0b1 << 15);      // Enable BSC controller
    PUT32(BSC_S, ((1 << 1) | (1 << 8) | (1 << 9)));      // Clear status
    dev_barrier();
}

