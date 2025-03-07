#include "rpi.h"
#include "i2c.h"

#define BSC0_BASE   0x20205000
#define BSC1_BASE   0x20804000
// #define BSC2_BASE   0x20805000

#define BSC_BASE    BSC1_BASE
#define BSC_C       (BSC_BASE + 0x0)   // Control register
#define BSC_S       (BSC_BASE + 0x4)   // Status register
#define BSC_DLEN    (BSC_BASE + 0x8)   // Data length register
#define BSC_A       (BSC_BASE + 0xC)   // Address register
#define BSC_FIFO    (BSC_BASE + 0x10)  // FIFO register
#define BSC_DIV     (BSC_BASE + 0x14)  // Clock divider register

void i2c_init(void) {
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

void i2c_init_clk_div(unsigned clk_div) {
    i2c_init();

    // Write to Clock Divider Register [BCM peripherals pg 34]
    // SCL = core_clk / CDIV, where core_clk is 150 MHz
    dev_barrier();
    PUT32(BSC_DIV, clk_div);
    dev_barrier();
}

int i2c_write(unsigned addr, uint8_t data[], unsigned nbytes) {
    unsigned int remaining_bytes = nbytes;
    unsigned int data_index = 0;

    dev_barrier();
    // Set the address
    PUT32(BSC_A, addr);
    // Clear the FIFO
    PUT32(BSC_C, (1 << 4));
    // Clear the status register
    PUT32(BSC_S, ((1 << 1) | (1 << 8) | (1 << 9)));
    // Set data length
    PUT32(BSC_DLEN, nbytes);

    // Start the transfer
    PUT32(BSC_C, (1 << 7) | (1 << 15));

    while(!(GET32(BSC_S) & (1 << 1))) {
        while(remaining_bytes > 0 && GET32(BSC_S) & (1 << 4)) { // Check if the FIFO can accept data
            // Write data to the FIFO
            PUT32(BSC_FIFO, data[data_index]);
            data_index++;
            remaining_bytes--;
        }
    }

    // Get status register
    uint32_t status = GET32(BSC_S);
    if (status & (1 << 8)) {
        putk("ERR ACK Error\n");
    }
    if (status & (1 << 9)) {
        putk("CLKT Clock Stretch Timeout\n");
    }
    if (remaining_bytes != 0) {
        putk("Remaining bytes not 0\n");
    }

    // Set done bit
    PUT32(BSC_S, (1 << 1));
    dev_barrier();
    return 0;
}

int i2c_read(unsigned addr, uint8_t data[], unsigned nbytes) {
    
    dev_barrier();

    unsigned int remaining_bytes = nbytes;
    unsigned int data_index = 0;
    // Set the address
    PUT32(BSC_A, addr);
    // Clear the FIFO
    PUT32(BSC_C, (1 << 4));
    // Clear the status register
    PUT32(BSC_S, ((1 << 1) | (1 << 8) | (1 << 9)));
    // Set data length
    PUT32(BSC_DLEN, nbytes);
    
    // Start the transfer
    PUT32(BSC_C, (1 << 7) | (1 << 15) | (1 << 0)); // Set the read bit

    while(!(GET32(BSC_S) & (1 << 1))) {
        while(remaining_bytes > 0 && GET32(BSC_S) & (1 << 5)) { // Check if the FIFO has data
            // Read data from the FIFO
            data[data_index] = GET32(BSC_FIFO);
            data_index++;
            remaining_bytes--;
        }
    }

    // Flush the FIFO
    while (remaining_bytes > 0 && GET32(BSC_S) & (1 << 5)) {
        data[data_index] = GET32(BSC_FIFO);
        data_index++;
        remaining_bytes--;
    }

    // Check for errors
    uint32_t status = GET32(BSC_S);
    if (status & (1 << 8)) {
        putk("ERR ACK Error\n");
    }
    if (status & (1 << 9)) {
        putk("CLKT Clock Stretch Timeout\n");
    }
    if (remaining_bytes != 0) {
        putk("Remaining bytes not 0\n");
    }

    // Set done bit
    PUT32(BSC_S, (1 << 1));
    dev_barrier();
    return 0;
}