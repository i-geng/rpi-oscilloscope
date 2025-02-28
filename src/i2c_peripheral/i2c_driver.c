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
#include "bit-support.h"

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
// static volatile RPI_i2c_t *i2c = (void *)0x20205000; // BSC0
static volatile RPI_i2c_t *i2c = (void *)0x20804000; // BSC1

// Checks for clock timeout and if any errors were detected.
// Returns 0 if no error, 1 if error.
int i2c_has_error() {
  dev_barrier();

  uint32_t status = GET32(i2c->status);
  printk("status=%b\n", status);

  // Check if there was a clock stretch timeout [BCM peripherals pg 31]
  // [9] -- CLKT Clock Stretch Timeout: 0 = No errors detected.
  uint32_t clkt = bit_get(status, 9);
  if (clkt == 1) {  // If timeout occurred, return 1
    dev_barrier();
    return 1;
  }

  // Check if there were any errors detected [BCM peripherals pg 31]
  // [8] -- ERR ACK Error: 0 = No errors detected.
  uint32_t err = bit_get(status, 8);
  if (err == 1) {   // If error detected, return 1
    dev_barrier();
    return 1;
  }

  dev_barrier();
  return 0;
}

// Returns 1 if i2c is enabled, 0 otherwise.
int i2c_is_enabled() {
  dev_barrier();

  uint32_t ctrl = GET32(i2c->control);
  uint32_t i2c_enabled = bit_get(ctrl, 15);
  
  dev_barrier();
  return i2c_enabled;
}

// Check if a transfer is active.
// Returns 1 if transfer is active, 0 otherwise.
int i2c_transfer_is_active() {
  dev_barrier();

  // Check S register [BCM peripherals pg 31]
  // [0] -- TA Transfer Active: 0 = Transfer not active. 1 = Transfer active.
  uint32_t status = GET32(i2c->status);
  uint32_t is_active = bit_get(status, 0);

  dev_barrier();
  return is_active;
}

// Check if a transfer is done.
// Returns 1 if transfer is done, 0 otherwise.
int i2c_transfer_is_done() {
  dev_barrier();

  // Check S register [BCM peripherals pg 31]
  // [1] -- DONE Transfer Done: 0 = Transfer not completed. 1 = Transfer complete.
  uint32_t status = GET32(i2c->status);
  uint32_t is_done = bit_get(status, 1);

  dev_barrier();
  return is_done;
}

// returns 1 if the hardware TX (output) FIFO has room
// for at least one byte. returns 0 otherwise.
int i2c_can_put8(void) {
  dev_barrier();

  // Status register [Broadcom peripherals pg 32]
  // [4] -- TXD FIFO can accept data: 0 = FIFO is full. 1 = FIFO has space for >= 1 byte.
  uint32_t status = GET32(i2c->status);
  uint32_t txd = bit_get(status, 4);

  dev_barrier();
  return txd;
}

// Puts one byte into the Transmit FIFO queue.
// Returns the byte that was written.
int i2c_put8(uint8_t c) {
  dev_barrier();

  // Block until the TX FIFO has space
  while (i2c_can_put8() == 0) {}

  // When writing, only bits [0:7] are taken
  PUT32(i2c->fifo, c);

  dev_barrier();
  return c;
}

// extend so this can fail.
int i2c_write(unsigned addr, uint8_t data[], unsigned nbytes) {
  dev_barrier();

  assert(i2c_is_enabled());

  // 1. Setup to start a transfer
  // Wait until transfer is not active
  while (i2c_transfer_is_active() == 1) {}

  // Read the status register
  uint32_t status = GET32(i2c->status);

  // Check that TX FIFO is empty [BCM peripherals pg 32]
  // [6] -- TXE FIFO Empty: 0 = FIFO is not empty. 1 = FIFO is empty.
  // uint32_t fifo_empty = bit_get(status, 6);
  // if (fifo_empty == 0) {  // If FIFO isn't empty, return 0
  //   putk("fifo not empty\n");
  //   return 0;
  // }

  // Check if any errors were detected
  if (i2c_has_error() == 1) {
    putk("i2c has error\n");
    return 0;
  }

  // Clear the DONE field [BCM peripherals pg 32]
  // [1] -- DONE Transfer Done: 0 = Transfer not completed. 1 = Transfer complete. Cleared by writing 1.
  uint32_t done = 0b1 << 1;
  PUT32(i2c->status, done);

  // Set the device address [BCM peripherals pg 33]
  // [6:0] -- Device address (should be 7 bits)
  PUT32(i2c->dev_addr, addr & 0b1111111);

  // Set the data length [BCM peripherals pg 32]
  // [15:0] -- Data length: writing to DLEN specifies the # of bytes to be transmitted/received
  PUT32(i2c->dlen, nbytes & 0xff);

  // Set the control register to "write" and start the transfer [BCM peripherals pg 30]
  // [0] -- 0 = WRITE packet transfer. 1 = READ packet transfer.
  // [7] -- Start Transfer: 0 = No action. 1 = Start a new transfer.
  uint32_t ctrl = GET32(i2c->control);
  assert(i2c_is_enabled());
  ctrl = bit_clr(ctrl, 0);    // specify this is a WRITE
  ctrl = bit_set(ctrl, 7);    // start a new transfer

  // Wait until the transfer has started
  while (i2c_transfer_is_active() == 0) {}

  // 2. Write the bytes
  for (size_t i = 0; i < nbytes; i++) {
    i2c_put8(data[i]);
  }

  // 3. Complete the transfer
  // Use status register to wait for DONE
  while (!i2c_transfer_is_done()) {}

  // Check that transfer is not active.
  assert(!i2c_transfer_is_active());

  // Check for any errors.
  if (i2c_has_error()) {
    putk("i2c has error\n");
    return 0;
  }

  return 1;
}

// extend so it returns failure.
int i2c_read(unsigned addr, uint8_t data[], unsigned nbytes) {
  todo("implement");
  return 1;
}

void i2c_init(void) {
  // Put a device barrier, since this is a BCM peripheral
  dev_barrier();

  // Setup GPIO SCL (pin 3) and SDA (pin2)
  // Configure SCL and SDA pins to use ALT0 function [BCM peripherals pg 102]
  uint32_t GPIO_SDA = 2;
  uint32_t GPIO_SCL = 3;
  gpio_set_function(GPIO_SDA, GPIO_FUNC_ALT0);
  gpio_set_function(GPIO_SCL, GPIO_FUNC_ALT0);
  dev_barrier();

  // Enable I2C by writing to C register [BCM peripherals pg 29]
  // [15] -- I2C Enable: 1 = BSC controller is enabled. 0 = BSC controller is disabled.
  // [5:4] -- CLEAR FIFO Clear: 00 = No action. x1 = Clear FIFO. 1x = Clear FIFO.
  // uint32_t ctrl = 0b1 << 15;
  uint32_t ctrl = (0b1 << 15) | (0b11 << 4);
  put32(&i2c->control, ctrl);
  dev_barrier();

  // Write to Clock Divider Register [BCM peripherals pg 34]
  // SCL = core_clk / CDIV, where core_clk is 150 MHz
  // We set CDIV = 1500, so that SCL will be 100 kHz (I2C standard mode)
  uint32_t cdiv = 1500;
  PUT32(&i2c->clock_div, cdiv);
  dev_barrier();

  // Clear the BSC status register; access S register [BCM peripherals pg 31]
  // [9] -- CLKT Clock Stretch Timeout: 0 = No errors detected. Cleared by writing 1 to the field.
  // [8] -- ERR ACK Error: 0 = No errors detected. Cleared by writing 1 to the field.
  // [1] -- DONE Transfer Done: 0 = Transfer not completed. 1 = Transfer complete. Cleared by writing 1.
  // uint32_t status = (0b1 << 9) | (0b1 << 8) | (0b1 << 1);
  uint32_t status = GET32(i2c->status);
  status = status << 22;
  status = status >> 22;
  status |= (0b1 << 9) | (0b1 << 8) | (0b1 << 1);
  put32(&i2c->status, status);
  dev_barrier();

  // Assert that there is no active transfer; check S register [BCM peripherals pg 31]
  // [0] -- TA Transfer Active: 0 = Transfer not active. 1 = Transfer active.
  assert(!i2c_transfer_is_active());
}

// shortest will be 130 for i2c accel.
void i2c_init_clk_div(unsigned clk_div) {
  i2c_init();

  // Write to Clock Divider Register [BCM peripherals pg 34]
  // SCL = core_clk / CDIV, where core_clk is 150 MHz
  dev_barrier();
  PUT32(&i2c->clock_div, clk_div);
  dev_barrier();
}