#ifndef __RPI_MULTI_DISPLAY_H__
#define __RPI_MULTI_DISPLAY_H__

#include "rpi.h"  // Make sure "rpi.h" is the first include!
#include "multi-i2c.h"

// Multi-display library to draw to multiple SSD1306
// displays that are placed horizontally next to each other.

// This library is based on the ADAfruit SSD1306 library:
// https://github.com/adafruit/Adafruit_SSD1306
// https://github.com/adafruit/Adafruit-GFX-Library

// This is declared here as extern and defined in standard-ascii-font.c
extern const unsigned char standard_ascii_font[];

enum {
  // Specify the number of displays
  NUM_DISPLAYS = 1,

  // Statistics for a single SSD1306 display
  DISPLAY_WIDTH = 128,
  DISPLAY_HEIGHT = 64,
  DISPLAY_BUFFER_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT / 8,

  // Multi-display statistics (depends on NUM_DISPLAYS)
  MULTI_DISPLAY_WIDTH = NUM_DISPLAYS * DISPLAY_WIDTH,
  MULTI_DISPLAY_HEIGHT = DISPLAY_HEIGHT,
  MULTI_DISPLAY_BUFFER_SIZE = NUM_DISPLAYS * DISPLAY_BUFFER_SIZE,
};

// Multi-display pixel buffer
static uint8_t multi_display_buffer[MULTI_DISPLAY_BUFFER_SIZE];

// Stats display pixel buffer
static uint8_t stats_display_buffer[DISPLAY_BUFFER_SIZE];

// Struct that describes a single display
typedef struct {
  // I2C address of display
  uint8_t device_address;

  // Function pointer to i2c_write() function
  int (*i2c_write_func)(unsigned, uint8_t *, unsigned);
} display_configuration_t;

// Create a display_configuration_t struct for each display
static display_configuration_t display_config_arr[NUM_DISPLAYS] = {
    {0x3C, i2c_write_BSC1},
    // {0x3D, i2c_write_BSC1},
};

// Create a display_configuration_t struct for the stats display
static display_configuration_t stats_display_config = {0x3C, i2c_write_BSC0};

// Struct that describes graph configuration
typedef struct {
  // Pixel margins around the graph
  uint8_t margin_left;
  uint8_t margin_right;
  uint8_t margin_top;
  uint8_t margin_bottom;

  // Ranges for x- and y-axes (real numbers, not pixel coordinates)
  int16_t x_axis_min;
  int16_t x_axis_max;
  float x_axis_span; // x_axis_span = x_axis_max - x_axis_min

  int16_t y_axis_min;
  int16_t y_axis_max;
  float y_axis_span; // y_axis_span = y_axis_max - y_axis_min

  // Where is the horizontal line for the graph x-axis?
  uint16_t y_horizontal;

  // Where is the vertical line for the graph y-axis?
  uint16_t x_vertical;

} graph_configuration_t;

// Graph configuration for multi-display
static graph_configuration_t graph_config = {
    .margin_left = 30,
    .margin_right = 3,
    .margin_bottom = 8,
    .margin_top = 3,

    .x_axis_min = 0,
    .x_axis_max = 10,

    .y_axis_min = -5,
    .y_axis_max = 5,
};

// Enum for monochrome display colors.
typedef enum {
  COLOR_WHITE,  // Draw white pixel (turn on)
  COLOR_BLACK,  // Draw black pixel (turn off)
  COLOR_INVERT, // Invert the pixel's original color
} color_t;

// Macro to read byte
#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

// Macro to swap two values
#ifndef SWAP
#define SWAP(x, y)                                                             \
  do {                                                                         \
    typeof(x) SWAP = x;                                                        \
    x = y;                                                                     \
    y = SWAP;                                                                  \
  } while (0)
#endif

// Initialize the multi-display. Requirement: I2C should have been initialized
// beforehand
void multi_display_init(void);

void stats_display_init(void);

// Initialize a single SSD1306 display
void single_display_init(display_configuration_t display_config);

void multi_display_send_byte(uint32_t index);
void multi_display_separate_buffers(void);

// Send display buffer to screen via I2C
// Must be called to actually update the display!
void multi_display_show(void);

void stats_display_show(void);

// Helper function to send a byte over I2C
void multi_display_send_command(uint8_t cmd,
                                display_configuration_t display_config);

// Clears the screen to black; no change until display_show() is called
void multi_display_clear(void);

void stats_display_clear(void);

// Fills the display completely with white
void multi_display_fill_white(void);

void stats_display_fill_white(void);

// Draw a pixel at coordinates (x, y) with specified color
// Convention: top left corner of screen is pixel (0, 0)
void multi_display_draw_pixel(uint16_t x, uint16_t y, color_t color);
void stats_display_draw_pixel(uint16_t x, uint16_t y, color_t color);

// Draw a horizontal line from (x_start, y) to (x_end, y), inclusive of both
// endpoins Convention: top left corner of screen is pixel (0, 0)
void multi_display_draw_horizontal_line(int16_t x_start, int16_t x_end,
                                        int16_t y, color_t color);

// Draw a vertical line from (y_start, x) to (y_end, x), inclusive of both
// endpoints Convention: top left corner of screen is pixel (0, 0)
void multi_display_draw_vertical_line(int16_t y_start, int16_t y_end, int16_t x,
                                      color_t color);
void stats_display_draw_vertical_line(int16_t y_start, int16_t y_end, int16_t x,
                                      color_t color);

// Draw an ASCII character at (x, y) with specified color
// Convention: top left corner of screen is pixel (0, 0)
void multi_display_draw_character(int16_t x, int16_t y, unsigned char c,
                                  color_t color);
void stats_display_draw_character(int16_t x, int16_t y, unsigned char c,
                                  color_t color);

void multi_display_draw_character_size(int16_t x, int16_t y, unsigned char c, 
                                       color_t color, uint8_t size_x, uint8_t size_y);
void stats_display_draw_character_size(int16_t x, int16_t y, unsigned char c, 
                                       color_t color, uint8_t size_x, uint8_t size_y);

void multi_display_draw_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, color_t color);
void stats_display_draw_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, color_t color);

void multi_display_configure_graph_axes(int16_t x_axis_min, int16_t x_axis_max,
                                        int16_t y_axis_min, int16_t y_axis_max);

void multi_display_configure_graph_margins(uint8_t margin_left,
                                           uint8_t margin_right,
                                           uint8_t margin_top,
                                           uint8_t margin_bottom);

void multi_display_update_graph_configuration(void);

void multi_display_draw_graph_axes(void);
void multi_display_draw_graph_tick(uint32_t label);

void multi_display_draw_graph_data(float *x_values, float *y_values, uint16_t N, color_t color);

void stats_display_draw_data(float amplitude, float frequency);

#endif