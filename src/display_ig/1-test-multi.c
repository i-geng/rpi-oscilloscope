#include "rpi.h"
#include "multi-display.h"
#include "i2c.h"

void notmain(void) {
  // Initialize I2C with some settling time
  delay_ms(100);
  i2c_init_clk_div(1500);
  delay_ms(100);

  // Initialize display
  multi_display_init();
  // multi_display_fill_white();

  // Some draw commands
  multi_display_draw_character(5, 20, 'A', COLOR_WHITE);
  multi_display_draw_character(230, 20, 'B', COLOR_WHITE);
  multi_display_draw_character(258, 45, 'C', COLOR_WHITE);

  // for (int16_t x = 0; x < MULTI_DISPLAY_WIDTH; x++) {
  //   multi_display_draw_pixel(x, 0.1 * x, COLOR_INVERT);
  // }

  // display_draw_horizontal_line(0, 50, 10, COLOR_INVERT);
  // display_draw_vertical_line(20, 64, 20, COLOR_INVERT);
  // display_draw_character(40, 40, '1', COLOR_WHITE);
  // display_draw_character(44, 40, '2', COLOR_WHITE);

  // multi_display_configure_graph_axes(0, 0, 0, 0);
  // multi_display_draw_graph_axes();
  // multi_display_draw_graph_data();

  // Must call display_show() to actually update the screen
  multi_display_show();

  while (1) {}
}