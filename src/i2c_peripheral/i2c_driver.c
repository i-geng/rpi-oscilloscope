/*
 * simplified i2c implementation --- no dma, no interrupts.  the latter
 * probably should get added.  the pi's we use can only access i2c1
 * so we hardwire everything in.
 *
 * datasheet starts at p28 in the broadcom pdf.
 *
 */

#include "rpi.h"
#include "libc/helper-macros.h"
#include "i2c_driver.h"

typedef struct {
  uint32_t control; // "C" register, p29
  uint32_t status;  // "S" register, p31

#define check_dlen(x) assert(((x) >> 16) == 0)
  uint32_t dlen; // p32. number of bytes to xmit, recv
                 // reading from dlen when TA=1
                 // or DONE=1 returns bytes still
                 // to recv/xmit.
                 // reading when TA=0 and DONE=0
                 // returns the last DLEN written.
                 // can be left over multiple pkts.

  // Today address's should be 7 bits.
#define check_dev_addr(x) assert(((x) >> 7) == 0)
  uint32_t dev_addr; // "A" register, p 33, device addr

  uint32_t fifo; // p33: only use the lower 8 bits.
#define check_clock_div(x) assert(((x) >> 16) == 0)
  uint32_t clock_div; // p34
  // we aren't going to use this: fun to mess w/ tho.
  uint32_t clock_delay;           // p34
  uint32_t clock_stretch_timeout; // broken on pi.
} RPI_i2c_t;

// offsets from table "i2c address map" p 28
_Static_assert(offsetof(RPI_i2c_t, control) == 0, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, status) == 0x4, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, dlen) == 0x8, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, dev_addr) == 0xc, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, fifo) == 0x10, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, clock_div) == 0x14, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, clock_delay) == 0x18, "wrong offset");

/*
 * There are three BSC masters inside BCM. The register addresses starts from
 *	 BSC0: 0x7E20_5000 (0x20205000)
 *	 BSC1: 0x7E80_4000
 *	 BSC2 : 0x7E80_5000 (0x20805000)
 * the PI can only use BSC1.
 */
static volatile RPI_i2c_t *i2c = (void *)0x20804000; // BSC1

// extend so this can fail.
int i2c_write(unsigned addr, uint8_t data[], unsigned nbytes) {
  todo("implement");
  return 1;
}

// extend so it returns failure.
int i2c_read(unsigned addr, uint8_t data[], unsigned nbytes) {
  todo("implement");
  return 1;
}

void i2c_init(void) {
  // todo("setup GPIO, setup i2c, sanity check results");
  // Put a device barrier, since this is a BCM peripheral
  dev_barrier();

  // Setup GPIO SCL1 (pin 5) and SDA1 (pin3)
  // Configure SCL1 and SDA1 pins to use ALT0 function [BCM peripherals pg 102]
  uint32_t GPIO_SCL1 = 5;
  uint32_t GPIO_SDA1 = 3;
  gpio_set_function(GPIO_SCL1, GPIO_FUNC_ALT0);
  gpio_set_function(GPIO_SDA1, GPIO_FUNC_ALT0);

  dev_barrier();

  // Enable I2C by writing to C register [BCM peripherals pg 29]
  // [15] -- I2C Enable: 1 = BSC controller is enabled. 0 = BSC controller is disabled.
  // [5:4] -- CLEAR FIFO Clear: 00 = No action. x1 = Clear FIFO. 1x = Clear FIFO.
  uint32_t i2c_enable = 0b1 << 15;
  i2c_enable |= 0b01 << 4;
  PUT32(i2c->control, i2c_enable);

  dev_barrier();

  // Write to Clock Divider Register [BCM peripherals pg 34]
  // SCL = core_clk / CDIV, where core_clk is 150 MHz
  // We set CDIV = 1500, so that SCL will be 100 kHz (I2C standard mode)
  uint32_t cdiv = 1500;
  PUT32(i2c->clock_div, cdiv);

  // Clear the BSC status register; access S register [BCM peripherals pg 31]
  // [9] -- CLKT Clock Stretch Timeout: 0 = No errors detected.
  // [8] -- ERR ACK Error: 0 = No errors detected.
  // [1] -- DONE Transfer Done: 0 = Transfer not completed. 1 = Transfer complete. Cleared by writing 1.
  // [0] -- TA Transfer Active: 0 = Transfer not active. 1 = Transfer active.
  uint32_t status = 0b1 << 1;
  PUT32(i2c->status, status);

  dev_barrier();

  // Assert that there is no active transfer; check S register [BCM peripherals pg 31]
  // [0] -- TA Transfer Active: 0 = Transfer not active. 1 = Transfer active.
  uint32_t val = GET32(i2c->status);
  assert((val & 0b1) == 0);

  dev_barrier();
}

// shortest will be 130 for i2c accel.
void i2c_init_clk_div(unsigned clk_div) {
  todo("same as init but set the clock divider");
}