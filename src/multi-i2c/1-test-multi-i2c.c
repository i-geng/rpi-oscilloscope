#include "rpi.h"
#include "multi-display.h"
#include "i2c.h"

void notmain(void) {
  // Initialize I2C with some settling time
  delay_ms(100);
  i2c_init();
  delay_ms(100);

  // Initialize display
  multi_display_init();

  multi_display_fill_white();
  multi_display_show();

  while (1) {}
}