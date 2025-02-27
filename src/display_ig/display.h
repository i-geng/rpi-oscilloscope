#ifndef __RPI_DISPLAY_H__
#define __RPI_DISPLAY_H__

typedef enum {
  COLOR_WHITE,
  COLOR_BLACK,
  COLOR_INVERT,
} color_t;

enum {
  DISPLAY_ADDRESS = 0X3C,
  DISPLAY_WIDTH = 128,
  DISPLAY_HEIGHT = 64,
  DISPLAY_BUFFER_SIZE = DISPLAY_WIDTH * ((DISPLAY_HEIGHT + 7) / 8),

  // Ranges for x- and y-axes (real numbers, not pixel coordinates)
  x_axis_min = 0,
  x_axis_max = 100,
  y_axis_min = 0,
  y_axis_max = 50,

  // Pixel margins around the graph
  left_margin = 5,
  right_margin = 0,
  top_margin = 0,
  bottom_margin = 5,

};

void display_init(void);

void display_send_command(uint8_t cmd);

void display_show(void);

void display_draw_pixel(uint16_t x, uint16_t y, color_t color);

void display_clear(void);

void display_fill_buffer(void);

void draw_horizontal_line(int16_t x_start, int16_t x_end, int16_t y, color_t color);

#endif