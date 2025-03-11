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
  i2c_init_clk_div(1500);
  delay_ms(100);

  // Initialize display
  multi_display_init();

  // convert_data_to_ascii(123.2);

  // multi_display_fill_white();

  // Some draw commands
  // multi_display_draw_character(35, 20, 'A', COLOR_WHITE);
  // multi_display_draw_character(230, 20, 'B', COLOR_WHITE);
  // multi_display_draw_character(258, 45, 'C', COLOR_WHITE);

  // for (int16_t x = 0; x < MULTI_DISPLAY_WIDTH; x++) {
  //   multi_display_draw_pixel(x, 0.1 * x, COLOR_INVERT);
  // }

  // display_draw_horizontal_line(0, 50, 10, COLOR_INVERT);
  // display_draw_vertical_line(20, 64, 20, COLOR_INVERT);
  // display_draw_character(40, 40, '1', COLOR_WHITE);
  // display_draw_character(44, 40, '2', COLOR_WHITE);

  multi_display_configure_graph_axes(0, 1000*1.2, 0, 10);

  float index[1000];
  float data[1000];

  for (int i = 0; i < 1000; i++) {
    index[i] = i;
    data[i] = i * 0.01;
  }

  multi_display_draw_graph_axes();
  multi_display_draw_graph_data(index, data, 1000, COLOR_WHITE);

  // float x_values[10] = {0, 1.2, 2.4, 3.6, 4.8, 6.0, 7.2, 8.4, 9.6, 10.8};
  // float y_values[10] = {4.997, 1.617, 1.617, 1.617, 0.016,
  //                       0.016, 1.085, 1.085, 4.223, 4.223};
  // multi_display_draw_graph_data(x_values, y_values, 10, COLOR_WHITE);

  // Must call display_show() to actually update the screen
  multi_display_show();

  while (1) {}
}