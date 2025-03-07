#include "rpi.h"
#include "display.h"
#include "i2c.h"

void notmain(void) {
  // Initialize I2C with some settling time
  delay_ms(100);
  i2c_init_clk_div(1500);
  delay_ms(100);

  // Initialize display
  display_init();

  // Some draw commands
  // for (int16_t x = 0; x < DISPLAY_HEIGHT; x++) {
  //   display_draw_pixel(x, x, COLOR_INVERT);
  // }

  // display_draw_horizontal_line(0, 50, 10, COLOR_INVERT);
  // display_draw_vertical_line(20, 64, 20, COLOR_INVERT);
  // display_draw_character(40, 40, '1', COLOR_WHITE);
  // display_draw_character(44, 40, '2', COLOR_WHITE);

  display_configure_graph_axes(0, 0, 0, 0);
  display_draw_graph_axes();
  display_draw_graph_data();

  // Must call display_show() to actually update the screen
  display_show();

  while (1) {}
}