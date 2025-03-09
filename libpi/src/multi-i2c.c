#include "rpi.h"
#include "multi-i2c.h"

#define BSC0_BASE   0x20205000
#define BSC1_BASE   0x20804000
// #define BSC2_BASE   0x20805000

#define BSC0_C       (BSC0_BASE + 0x0)   // Control register
#define BSC0_S       (BSC0_BASE + 0x4)   // Status register
#define BSC0_DLEN    (BSC0_BASE + 0x8)   // Data length register
#define BSC0_A       (BSC0_BASE + 0xC)   // Address register
#define BSC0_FIFO    (BSC0_BASE + 0x10)  // FIFO register
#define BSC0_DIV     (BSC0_BASE + 0x14)  // Clock divider register

#define BSC1_C       (BSC1_BASE + 0x0)   // Control register
#define BSC1_S       (BSC1_BASE + 0x4)   // Status register
#define BSC1_DLEN    (BSC1_BASE + 0x8)   // Data length register
#define BSC1_A       (BSC1_BASE + 0xC)   // Address register
#define BSC1_FIFO    (BSC1_BASE + 0x10)  // FIFO register
#define BSC1_DIV     (BSC1_BASE + 0x14)  // Clock divider register

void i2c_init_BSC0(void) {
    dev_barrier();
    // Set GPIO pins 0 and 1 for I2C function (alt function 0)
    gpio_set_function(0, 4);  // SDA
    gpio_set_function(1, 4);  // SCL
    dev_barrier();

    // Reset BSC clock, controller, and status
    PUT32(BSC0_DIV, 0x5dc);
    PUT32(BSC0_C, 0b1 << 15);      // Enable BSC controller
    PUT32(BSC0_S, ((1 << 1) | (1 << 8) | (1 << 9)));      // Clear status
    dev_barrier();
}

void i2c_init_BSC1(void) {
    dev_barrier();
    // Set GPIO pins 2 and 3 for I2C function (alt function 0)
    gpio_set_function(2, 4);  // SDA
    gpio_set_function(3, 4);  // SCL
    dev_barrier();

    // Reset BSC clock, controller, and status
    PUT32(BSC1_DIV, 0x5dc);
    PUT32(BSC1_C, 0b1 << 15);      // Enable BSC controller
    PUT32(BSC1_S, ((1 << 1) | (1 << 8) | (1 << 9)));      // Clear status
    dev_barrier();
}

void i2c_init_clk_div_BSC0(unsigned clk_div) {
    i2c_init_BSC0();

    // Write to Clock Divider Register [BCM peripherals pg 34]
    // SCL = core_clk / CDIV, where core_clk is 150 MHz
    dev_barrier();
    PUT32(BSC0_DIV, clk_div);
    dev_barrier();
}

void i2c_init_clk_div_BSC1(unsigned clk_div) {
    i2c_init_BSC1();

    // Write to Clock Divider Register [BCM peripherals pg 34]
    // SCL = core_clk / CDIV, where core_clk is 150 MHz
    dev_barrier();
    PUT32(BSC1_DIV, clk_div);
    dev_barrier();
}

int i2c_write_BSC0(unsigned addr, uint8_t data[], unsigned nbytes) {
    unsigned int remaining_bytes = nbytes;
    unsigned int data_index = 0;

    dev_barrier();
    // Set the address
    PUT32(BSC0_A, addr);
    // Clear the FIFO
    PUT32(BSC0_C, (1 << 4));
    // Clear the status register
    PUT32(BSC0_S, ((1 << 1) | (1 << 8) | (1 << 9)));
    // Set data length
    PUT32(BSC0_DLEN, nbytes);

    // Start the transfer
    PUT32(BSC0_C, (1 << 7) | (1 << 15));

    while(!(GET32(BSC0_S) & (1 << 1))) {
        while(remaining_bytes > 0 && GET32(BSC0_S) & (1 << 4)) { // Check if the FIFO can accept data
            // Write data to the FIFO
            PUT32(BSC0_FIFO, data[data_index]);
            data_index++;
            remaining_bytes--;
        }
    }

    // Get status register
    uint32_t status = GET32(BSC0_S);
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
    PUT32(BSC0_S, (1 << 1));
    dev_barrier();
    return 0;
}

int i2c_write_BSC1(unsigned addr, uint8_t data[], unsigned nbytes) {
    unsigned int remaining_bytes = nbytes;
    unsigned int data_index = 0;

    dev_barrier();
    // Set the address
    PUT32(BSC1_A, addr);
    // Clear the FIFO
    PUT32(BSC1_C, (1 << 4));
    // Clear the status register
    PUT32(BSC1_S, ((1 << 1) | (1 << 8) | (1 << 9)));
    // Set data length
    PUT32(BSC1_DLEN, nbytes);

    // Start the transfer
    PUT32(BSC1_C, (1 << 7) | (1 << 15));

    while(!(GET32(BSC1_S) & (1 << 1))) {
        while(remaining_bytes > 0 && GET32(BSC1_S) & (1 << 4)) { // Check if the FIFO can accept data
            // Write data to the FIFO
            PUT32(BSC1_FIFO, data[data_index]);
            data_index++;
            remaining_bytes--;
        }
    }

    // Get status register
    uint32_t status = GET32(BSC1_S);
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
    PUT32(BSC1_S, (1 << 1));
    dev_barrier();
    return 0;
}

int i2c_read_BSC0(unsigned addr, uint8_t data[], unsigned nbytes) {
    
    dev_barrier();

    unsigned int remaining_bytes = nbytes;
    unsigned int data_index = 0;
    // Set the address
    PUT32(BSC0_A, addr);
    // Clear the FIFO
    PUT32(BSC0_C, (1 << 4));
    // Clear the status register
    PUT32(BSC0_S, ((1 << 1) | (1 << 8) | (1 << 9)));
    // Set data length
    PUT32(BSC0_DLEN, nbytes);
    
    // Start the transfer
    PUT32(BSC0_C, (1 << 7) | (1 << 15) | (1 << 0)); // Set the read bit

    while(!(GET32(BSC0_S) & (1 << 1))) {
        while(remaining_bytes > 0 && GET32(BSC0_S) & (1 << 5)) { // Check if the FIFO has data
            // Read data from the FIFO
            data[data_index] = GET32(BSC0_FIFO);
            data_index++;
            remaining_bytes--;
        }
    }

    // Flush the FIFO
    while (remaining_bytes > 0 && GET32(BSC0_S) & (1 << 5)) {
        data[data_index] = GET32(BSC0_FIFO);
        data_index++;
        remaining_bytes--;
    }

    // Check for errors
    uint32_t status = GET32(BSC0_S);
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
    PUT32(BSC0_S, (1 << 1));
    dev_barrier();
    return 0;
}

int i2c_read_BSC1(unsigned addr, uint8_t data[], unsigned nbytes) {
    
    dev_barrier();

    unsigned int remaining_bytes = nbytes;
    unsigned int data_index = 0;
    // Set the address
    PUT32(BSC1_A, addr);
    // Clear the FIFO
    PUT32(BSC1_C, (1 << 4));
    // Clear the status register
    PUT32(BSC1_S, ((1 << 1) | (1 << 8) | (1 << 9)));
    // Set data length
    PUT32(BSC1_DLEN, nbytes);
    
    // Start the transfer
    PUT32(BSC1_C, (1 << 7) | (1 << 15) | (1 << 0)); // Set the read bit

    while(!(GET32(BSC1_S) & (1 << 1))) {
        while(remaining_bytes > 0 && GET32(BSC1_S) & (1 << 5)) { // Check if the FIFO has data
            // Read data from the FIFO
            data[data_index] = GET32(BSC1_FIFO);
            data_index++;
            remaining_bytes--;
        }
    }

    // Flush the FIFO
    while (remaining_bytes > 0 && GET32(BSC1_S) & (1 << 5)) {
        data[data_index] = GET32(BSC1_FIFO);
        data_index++;
        remaining_bytes--;
    }

    // Check for errors
    uint32_t status = GET32(BSC1_S);
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
    PUT32(BSC1_S, (1 << 1));
    dev_barrier();
    return 0;
}