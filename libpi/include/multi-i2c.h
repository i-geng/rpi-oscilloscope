#include "rpi.h"

void i2c_init_BSC0(void);
void i2c_init_BSC1(void);

// shortest will be 130 for i2c accel.
void i2c_init_clk_div_BSC0(unsigned clk_div);
void i2c_init_clk_div_BSC1(unsigned clk_div);

// write <nbytes> of <data> to i2c device address <addr>
int i2c_write_BSC0(unsigned addr, uint8_t data[], unsigned nbytes);
int i2c_write_BSC1(unsigned addr, uint8_t data[], unsigned nbytes);

// read <nbytes> of <data> from i2c device address <addr>
int i2c_read_BSC0(unsigned addr, uint8_t data[], unsigned nbytes);
int i2c_read_BSC1(unsigned addr, uint8_t data[], unsigned nbytes);