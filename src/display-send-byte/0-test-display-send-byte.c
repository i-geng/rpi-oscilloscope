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
  multi_display_clear();

  stats_display_init();
  stats_display_clear();

  float x_data[100];
  float y_data[100];
  for (int i = 0; i < 100; i++) {
    x_data[i] = i;
    y_data[i] = i * 0.5;
  }

  for (int i = 0; i < 100; i++) {
    multi_display_draw_graph_tick(i);
    multi_display_draw_graph_data(x_data, y_data, 100, COLOR_WHITE);
    stats_display_draw_data(i * 0.1, i * 0.15);

    uint32_t nbytes = 8;

    multi_display_separate_buffers();
    for (int index = 0; index < DISPLAY_BUFFER_SIZE; index += nbytes) {
      uint32_t start_time = timer_get_usec();
      multi_display_send_nbytes(index, nbytes);
      uint32_t end_time = timer_get_usec();
      printk("time per chunk = %d us\n", end_time - start_time);
    }

    for (int index = 0; index < DISPLAY_BUFFER_SIZE; index += nbytes) {
      uint32_t start_time = timer_get_usec();
      stats_display_send_nbytes(index, nbytes);
      uint32_t end_time = timer_get_usec();
      printk("time per chunk = %d us\n", end_time - start_time);
    }
    
    // delay_ms(500);
    multi_display_clear();
    stats_display_clear();
  }

  while (1) {}
}