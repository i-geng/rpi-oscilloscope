#include "rpi.h"
#include "multi-i2c.h"
#include "multi-display.h"

void notmain(void) {
  // Initialize both I2C channels with some settling time
  delay_ms(100);
  i2c_init_BSC0();
  delay_ms(100);
  i2c_init_BSC1();
  delay_ms(100);

  // Initialize displays
  multi_display_init();
  stats_display_init();
  multi_display_clear();
  stats_display_clear();

  float x_data[100];
  float y_data[100];
  for (int i = 0; i < 100; i++) {
    x_data[i] = i;
    y_data[i] = i * 0.5;
  }
  

  for (int i = 0; i < 100; i++) {
    multi_display_draw_graph_tick(i);
    multi_display_draw_graph_data(x_data, y_data, 50, COLOR_WHITE);
    stats_display_draw_data(i * 0.1, i * 0.15);

    multi_display_show();
    stats_display_show();

    delay_ms(100);
    multi_display_clear();
    stats_display_clear();
  }

  while (1) {}
}