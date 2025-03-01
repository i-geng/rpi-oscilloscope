#ifndef __RPI_DISPLAY_H__
#define __RPI_DISPLAY_H__

// This library is based on the ADAfruit SSD1306 library
// https://github.com/adafruit/Adafruit_SSD1306/tree/master
// https://github.com/adafruit/Adafruit-GFX-Library

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
};

typedef struct {
  // Pixel margins around the graph
  uint8_t margin_left;
  uint8_t margin_right;
  uint8_t margin_top;
  uint8_t margin_bottom;

  // Ranges for x- and y-axes (real numbers, not pixel coordinates)
  int16_t x_axis_min;
  int16_t x_axis_max;
  float x_axis_span;
  
  int16_t y_axis_min;
  int16_t y_axis_max;
  float y_axis_span;

  // Where is the horizontal line for the graph x-axis?
  uint16_t y_horizontal;

  // Where is the vertical line for the graph y-axis?
  uint16_t x_vertical;

} display_graph_configuration_t;

// Initialize the display. Requirement: I2C should have been initialized beforehand
void display_init(void);

// Send display buffer to screen via I2C
// Must be called to actually update the display!
void display_show(void);

// Helper function to send a byte over I2C
void display_send_command(uint8_t cmd);

// Clears the screen to black; no change until display_show() is called
void display_clear(void);

// Fills the display completely with white
void display_fill_white(void);

// Draw a pixel at coordinates (x, y) with specified color
// Convention: top left corner of screen is pixel (0, 0)
void display_draw_pixel(uint16_t x, uint16_t y, color_t color);

// Draw a horizontal line from (x_start, y) to (x_end, y), inclusive of both endpoins
// Convention: top left corner of screen is pixel (0, 0)
void display_draw_horizontal_line(int16_t x_start, int16_t x_end, int16_t y, color_t color);

// Draw a vertical line from (y_start, x) to (y_end, x), inclusive of both endpoints
// Convention: top left corner of screen is pixel (0, 0)
void display_draw_vertical_line(int16_t y_start, int16_t y_end, int16_t x, color_t color);

// Draw an ASCII character at (x, y) with specified color
// Convention: top left corner of screen is pixel (0, 0)
void display_draw_character(int16_t x, int16_t y, unsigned char c, color_t color);

void display_configure_graph_axes(int16_t x_min, int16_t x_max, int16_t y_min, int16_t y_max);

void display_draw_graph_axes(void);

void display_draw_graph_data(void);

#endif