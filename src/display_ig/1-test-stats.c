#include "rpi.h"
#include "multi-display.h"
#include "i2c.h"

void convert_data_to_ascii(int data) {
  char buffer[10];
  snprintk(buffer, sizeof(buffer), "%d  ", data);

  for (size_t i = 1; i <= strlen(buffer); i++) {
    multi_display_draw_character(50 - 5 * i,
                                 32,
                                 buffer[strlen(buffer) - i], COLOR_WHITE);
  }
}

void notmain(void) {
  // Initialize I2C with some settling time
  delay_ms(100);
  i2c_init();
  delay_ms(100);

  // Initialize display
  multi_display_init();

  // multi_display_draw_character(50, 20, 1, COLOR_WHITE);

  // multi_display_draw_fill_rect(15, 20, 15, 10, COLOR_WHITE);
  multi_display_draw_character_size(50, 20, 1, COLOR_WHITE, 4, 2);

  // Must call display_show() to actually update the screen
  multi_display_show();

  while (1) {}
}