#include "rpi.h"
#include "i2c_driver.h"

void notmain(void) {
  delay_ms(100); // allow time for i2c/device to boot up.
  i2c_init();
  delay_ms(100); // allow time for i2c/dev to settle after init.

  // Using the SSD1306's typical I2C address
  uint8_t display_addr = 0x3C;

  // Perform a dummy write with no register, just to see if the device acknowledges.
  // The SSD1306 expects a control byte before data. We'll send a command control byte (0x00).
  uint8_t control_byte = 0x00;
  int ret = i2c_write(display_addr, &control_byte, 1);
  if (ret < 0)
    panic("I2C probe failed: device at 0x%x did not acknowledge\n", display_addr);
  else
    printk("SUCCESS: SSD1306 at address 0x%x acknowledged our probe\n", display_addr);
}
