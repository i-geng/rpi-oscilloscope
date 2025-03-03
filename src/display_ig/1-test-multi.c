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
  multi_display_draw_character(35, 20, 'A', COLOR_WHITE);
  multi_display_draw_character(230, 20, 'B', COLOR_WHITE);
  multi_display_draw_character(258, 45, 'C', COLOR_WHITE);

  // for (int16_t x = 0; x < MULTI_DISPLAY_WIDTH; x++) {
  //   multi_display_draw_pixel(x, 0.1 * x, COLOR_INVERT);
  // }

  // display_draw_horizontal_line(0, 50, 10, COLOR_INVERT);
  // display_draw_vertical_line(20, 64, 20, COLOR_INVERT);
  // display_draw_character(40, 40, '1', COLOR_WHITE);
  // display_draw_character(44, 40, '2', COLOR_WHITE);

  multi_display_draw_graph_axes();

  float x_values[10] = {0, 1.2, 2.4, 3.6, 4.8, 6.0, 7.2, 8.4, 9.6, 10.8};
  float y_values[10] = {4.997, 1.617, 1.617, 1.617, 0.016,
                        0.016, 1.085, 1.085, 4.223, 4.223};
  multi_display_draw_graph_data(x_values, y_values, 10, COLOR_WHITE);

  // Must call display_show() to actually update the screen
  multi_display_show();

  while (1) {}
}