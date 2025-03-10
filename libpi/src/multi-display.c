#include "rpi.h"  // Make sure "rpi.h" is the first include!
#include "multi-display.h"

// This library is based on the ADAfruit SSD1306 library:
// https://github.com/adafruit/Adafruit_SSD1306
// https://github.com/adafruit/Adafruit-GFX-Library

// Initialize the display. Requirement: I2C should have been initialized
// beforehand
void multi_display_init(void) {
  // Initialize all displays
  for (int i = 0; i < NUM_DISPLAYS; i++) {
    single_display_init(display_config_arr[i]);
  }

  // Update graph configuration (precomputes some scaling constants)
  multi_display_update_graph_configuration();

  // Clear the multi-display to black
  multi_display_clear();
  multi_display_show();
}

void stats_display_init(void) {
  single_display_init(stats_display_config);
  stats_display_clear();
  stats_display_show();
}

// Helper function to send a byte over I2C to a particular function
void multi_display_send_command(uint8_t cmd,
                                display_configuration_t display_config) {
  uint8_t cmd_buf[2] = {0x00, cmd};
  display_config.i2c_write_func(display_config.device_address, cmd_buf, 2);
}

void single_display_init(display_configuration_t display_config) {
  // Display initialization flow [SSD1306 datasheet pg 64]
  // 0. Turn the display off to be safe [SSD1306 pg 28]
  multi_display_send_command(0xAE, display_config);

  // 1. Set multiplex ratio
  multi_display_send_command(0xA8, display_config);
  // use offset 63 because display is 64 pixels tall
  multi_display_send_command(0x3F, display_config);

  // 2. Set display offset [SSD1306 pg 37]
  multi_display_send_command(0xD3, display_config); // double byte command
  // map display start line to COM0 [SSD1306 pg 37]
  multi_display_send_command(0x00, display_config);
  // 3. Set display start line; RAM row 0 is mapped to COM0 [SSD1306 pg 36]
  multi_display_send_command(0x40, display_config);

  // 4. Set segment re-map; column addr 0 is mapped to SEG0 [SSD1306 pg 36]
  multi_display_send_command(0xA0, display_config);

  // 5. Set COM output scan direction; COM normal [SSD1306 pg 38]
  multi_display_send_command(0xC8, display_config);

  // 6. Set COM pins hardware configuration [SSD1306 pg 40]
  multi_display_send_command(0xDA, display_config);
  // [SSD1206 pg 11] diagram says to use 0x12
  multi_display_send_command(0x12, display_config);

  // 7. Set contrast control [SSD1306 pg 36]
  multi_display_send_command(0x81, display_config); // double byte command
  // the chip has 256 contrast steps from 00h to FFH
  // recommended value seems to be 127 (middle of the range)
  multi_display_send_command(0x7F, display_config);

  // 8. Display output according to GDDRAM contents [SSD1306 pg 37]
  multi_display_send_command(0xA4, display_config);

  // 9. Set normal or inverse display [SSD1306 pg 37]
  multi_display_send_command(0xA6, display_config); // normal
  // display_send_command(0xA7);  // inverse

  // 10. Set display clock divide ratio/oscillator frequency [SSD1306 pg 40]
  multi_display_send_command(0xD5, display_config); // double byte command
  // default value formula on [pg 22]
  multi_display_send_command(0x80, display_config);

  // 11. Enable charge pump regulator [SSD1306 pg 62]
  multi_display_send_command(0x8D, display_config); // double byte command
  multi_display_send_command(0x14, display_config); // enable charge pump

  // 12. Specify HORIZONTAL addressing mode [SSD1306 pg 35]
  // display_send_command(0x20);
  // display_send_command(0x02);   // page addressing mode [SSD1306 pg 34]
  multi_display_send_command(0x20, display_config);
  // horizontal addressing mode
  multi_display_send_command(0x00, display_config);
  multi_display_send_command(0x21, display_config); // set column address
  multi_display_send_command(0x00, display_config); // start at column 0
  multi_display_send_command(0x7F, display_config); // end at column 127
  multi_display_send_command(0x22, display_config); // set page address
  multi_display_send_command(0x00, display_config); // start at page 0
  multi_display_send_command(0x07, display_config); // end at page 7

  // 13. Display on [SSD1306 pg 62]
  multi_display_send_command(0xAF, display_config);

  // TODO: need to clear single display? will there be random noise?
}

// Send display buffer to screen via I2C
// Must be called to actually update the display!
void multi_display_show(void) {
  uint8_t display_buffers[NUM_DISPLAYS][1 + DISPLAY_BUFFER_SIZE];

  // Iterate over each row of the multi-display
  for (int row = 0; row < (DISPLAY_HEIGHT / 8); row++) {
    int multi_row_offset = row * MULTI_DISPLAY_WIDTH;

    // Iterate over all the displays
    for (int d = 0; d < NUM_DISPLAYS; d++) {
      int d_row_offset = row * DISPLAY_WIDTH;
      int d_col_offset = d * DISPLAY_WIDTH;

      // Copy the portion for this display
      // Important: index is (NUM_DISPLAYS - d - 1)
      // because buffer is copied onto screen from right-to-left, not
      // left-to-right
      memcpy(&display_buffers[NUM_DISPLAYS - d - 1][1 + d_row_offset],
             &multi_display_buffer[multi_row_offset + d_col_offset],
             DISPLAY_WIDTH);
    }
  }

  // Send each display buffer to its corresponding screen
  for (int d = 0; d < NUM_DISPLAYS; d++) {
    // Set first byte to 0x40 (control byte)
    display_buffers[d][0] = 0x40;
    display_config_arr[d].i2c_write_func(display_config_arr[d].device_address,
                                         display_buffers[d],
                                         1 + DISPLAY_BUFFER_SIZE);
  }
}

void stats_display_show(void) {
  // Send the stats display buffer to the correct screen
  uint8_t stats_buffer[1 + DISPLAY_BUFFER_SIZE];
  stats_buffer[0] = 0x40;
  memcpy(&stats_buffer[1], stats_display_buffer, DISPLAY_BUFFER_SIZE);
  stats_display_config.i2c_write_func(stats_display_config.device_address,
                                      stats_buffer,
                                      1 + DISPLAY_BUFFER_SIZE);
}

// Clears the screen to black; no change until display_show() is called
void multi_display_clear(void) {
  memset(multi_display_buffer, 0, sizeof(multi_display_buffer));
}

// Clears the screen to black; no change until display_show() is called
void stats_display_clear(void) {
  memset(stats_display_buffer, 0, sizeof(stats_display_buffer));
}

// Fills the display completely with white
void multi_display_fill_white(void) {
  memset(multi_display_buffer, 255, sizeof(multi_display_buffer));
}

void stats_display_fill_white(void) {
  memset(stats_display_buffer, 255, sizeof(stats_display_buffer));
}

// Draw a pixel at coordinates (x, y) with specified color
// Convention: top left corner of screen is pixel (0, 0)
void multi_display_draw_pixel(uint16_t x, uint16_t y, color_t color) {
  x = MULTI_DISPLAY_WIDTH - x - 1;
  switch (color) {
  case COLOR_WHITE:
    multi_display_buffer[(y / 8) * MULTI_DISPLAY_WIDTH + x] |= (1 << (y & 7));
    break;
  case COLOR_BLACK:
    multi_display_buffer[(y / 8) * MULTI_DISPLAY_WIDTH + x] &= ~(1 << (y & 7));
    break;
  case COLOR_INVERT:
    multi_display_buffer[(y / 8) * MULTI_DISPLAY_WIDTH + x] ^= (1 << (y & 7));
    break;
  }
}

void stats_display_draw_pixel(uint16_t x, uint16_t y, color_t color) {
  x = DISPLAY_WIDTH - x - 1;
  switch (color) {
  case COLOR_WHITE:
    stats_display_buffer[(y / 8) * DISPLAY_WIDTH + x] |= (1 << (y & 7));
    break;
  case COLOR_BLACK:
   stats_display_buffer[(y / 8) * DISPLAY_WIDTH + x] &= ~(1 << (y & 7));
    break;
  case COLOR_INVERT:
    stats_display_buffer[(y / 8) * DISPLAY_WIDTH + x] ^= (1 << (y & 7));
    break;
  }
}

// Draw a horizontal line from (x_start, y) to (x_end, y), inclusive of both
// endpoints. Convention: top left corner of screen is pixel (0, 0)
void multi_display_draw_horizontal_line(int16_t x_start, int16_t x_end,
                                        int16_t y, color_t color) {

  // Verify x_start is less than x_end
  if (x_start >= x_end) return;

  // Verify y is valid
  if (y < 0 || y >= MULTI_DISPLAY_HEIGHT) return;

  // Clamp x_start to 0, clamp x_end to (MULTI_DISPLAY_WIDTH - 1)
  x_start = x_start < 0 ? 0 : x_start;
  x_end = x_end > MULTI_DISPLAY_WIDTH - 1 ? MULTI_DISPLAY_WIDTH - 1 : x_end;

  // Transformation ensures top left corner of screen is pixel (0, 0)
  x_start = MULTI_DISPLAY_WIDTH - x_start - 1;
  x_end = MULTI_DISPLAY_WIDTH - x_end - 1;
  SWAP(x_start, x_end);

  // Compute the width of the line in pixels
  int16_t w = x_end - x_start;
  uint8_t *p_buf =
      &multi_display_buffer[(y / 8) * MULTI_DISPLAY_WIDTH + x_start];
  uint8_t mask = 1 << (y & 7);

  switch (color) {
  case COLOR_WHITE:
    while (w--) {
      *p_buf++ |= mask;
    };
    break;
  case COLOR_BLACK:
    mask = ~mask;
    while (w--) {
      *p_buf++ &= mask;
    };
    break;
  case COLOR_INVERT:
    while (w--) {
      *p_buf++ ^= mask;
    };
    break;
  }
}

// Draw a vertical line from (y_start, x) to (y_end, x), inclusive of both
// endpoints. Convention: top left corner of screen is pixel (0, 0)
void multi_display_draw_vertical_line(int16_t y_start, int16_t y_end, int16_t x,
                                      color_t color) {

  // Verify y_start is less than y_end
  if (y_start >= y_end) return;

  // Verify that x is valid
  if (x < 0 || x >= MULTI_DISPLAY_WIDTH) return;

  // Clamp y_start to 0 and y_end to (MULTI_DISPLAY_HEIGHT - 1)
  y_start = y_start < 0 ? 0 : y_start;
  y_end = y_end > MULTI_DISPLAY_HEIGHT - 1 ? MULTI_DISPLAY_HEIGHT - 1 : y_end;

  // Transformation to make sure top left corner of screen is pixel (0, 0)
  x = MULTI_DISPLAY_WIDTH - x - 1;

  // This display doesn't need ints for coordinates,
  // use local byte registers for faster juggling
  uint8_t y = y_start, h = y_end - y_start;
  uint8_t *p_buf = &multi_display_buffer[(y / 8) * MULTI_DISPLAY_WIDTH + x];

  // Do the first partial byte, if necessary - this requires some masking
  uint8_t mod = (y & 7);
  if (mod) {
    // Mask off the high n bits we want to set
    mod = 8 - mod;
    uint8_t mask = ~(0xFF >> mod);

    // Adjust the mask if we're not going to reach the end of this byte
    if (h < mod) {
      mask &= (0XFF >> (mod - h));
    }

    switch (color) {
    case COLOR_WHITE:
      *p_buf |= mask;
      break;
    case COLOR_BLACK:
      *p_buf &= ~mask;
      break;
    case COLOR_INVERT:
      *p_buf ^= mask;
      break;
    }
    p_buf += MULTI_DISPLAY_WIDTH;
  }

  if (h >= mod) { // More to go?
    h -= mod;
    // Write solid bytes while we can - effectively 8 rows at a time
    if (h >= 8) {
      if (color == COLOR_INVERT) {
        // Separate copy of the code so we don't impact performance of
        // black/white write version with an extra comparison per loop
        do {
          *p_buf ^= 0xFF;               // Invert byte
          p_buf += MULTI_DISPLAY_WIDTH; // Advance pointer 8 rows
          h -= 8;                       // Subtract 8 rows from height
        } while (h >= 8);
      } else {
        // store a local value to work with
        uint8_t val = (color != COLOR_BLACK) ? 255 : 0;
        do {
          *p_buf = val;                 // Set byte
          p_buf += MULTI_DISPLAY_WIDTH; // Advance pointer 8 rows
          h -= 8;                       // Subtract 8 rows from height
        } while (h >= 8);
      }
    }

    if (h) { // Do the final partial byte, if necessary
      mod = h & 7;
      // This time we want to mask the low bits of the byte,
      // vs the high bits we did above
      uint8_t mask = (1 << mod) - 1;
      switch (color) {
      case COLOR_WHITE:
        *p_buf |= mask;
        break;
      case COLOR_BLACK:
        *p_buf &= ~mask;
        break;
      case COLOR_INVERT:
        *p_buf ^= mask;
        break;
      }
    }
  }
}

void stats_display_draw_vertical_line(int16_t y_start, int16_t y_end, int16_t x,
                                      color_t color) {

  // Verify y_start is less than y_end
  if (y_start >= y_end) return;

  // Verify that x is valid
  if (x < 0 || x >= DISPLAY_WIDTH) return;

  // Clamp y_start to 0 and y_end to (MULTI_DISPLAY_HEIGHT - 1)
  y_start = y_start < 0 ? 0 : y_start;
  y_end = y_end > DISPLAY_HEIGHT - 1 ? DISPLAY_HEIGHT - 1 : y_end;

  // Transformation to make sure top left corner of screen is pixel (0, 0)
  x = DISPLAY_WIDTH - x - 1;

  // This display doesn't need ints for coordinates,
  // use local byte registers for faster juggling
  uint8_t y = y_start, h = y_end - y_start;
  uint8_t *p_buf = &stats_display_buffer[(y / 8) * DISPLAY_WIDTH + x];

  // Do the first partial byte, if necessary - this requires some masking
  uint8_t mod = (y & 7);
  if (mod) {
    // Mask off the high n bits we want to set
    mod = 8 - mod;
    uint8_t mask = ~(0xFF >> mod);

    // Adjust the mask if we're not going to reach the end of this byte
    if (h < mod) {
      mask &= (0XFF >> (mod - h));
    }

    switch (color) {
    case COLOR_WHITE:
      *p_buf |= mask;
      break;
    case COLOR_BLACK:
      *p_buf &= ~mask;
      break;
    case COLOR_INVERT:
      *p_buf ^= mask;
      break;
    }
    p_buf += DISPLAY_WIDTH;
  }

  if (h >= mod) { // More to go?
    h -= mod;
    // Write solid bytes while we can - effectively 8 rows at a time
    if (h >= 8) {
      if (color == COLOR_INVERT) {
        // Separate copy of the code so we don't impact performance of
        // black/white write version with an extra comparison per loop
        do {
          *p_buf ^= 0xFF;               // Invert byte
          p_buf += DISPLAY_WIDTH;       // Advance pointer 8 rows
          h -= 8;                       // Subtract 8 rows from height
        } while (h >= 8);
      } else {
        // store a local value to work with
        uint8_t val = (color != COLOR_BLACK) ? 255 : 0;
        do {
          *p_buf = val;                 // Set byte
          p_buf += DISPLAY_WIDTH; // Advance pointer 8 rows
          h -= 8;                       // Subtract 8 rows from height
        } while (h >= 8);
      }
    }

    if (h) { // Do the final partial byte, if necessary
      mod = h & 7;
      // This time we want to mask the low bits of the byte,
      // vs the high bits we did above
      uint8_t mask = (1 << mod) - 1;
      switch (color) {
      case COLOR_WHITE:
        *p_buf |= mask;
        break;
      case COLOR_BLACK:
        *p_buf &= ~mask;
        break;
      case COLOR_INVERT:
        *p_buf ^= mask;
        break;
      }
    }
  }
}

// Draw an ASCII character at (x, y) with specified color
// Convention: top left corner of screen is pixel (0, 0)
void multi_display_draw_character(int16_t x, int16_t y, unsigned char c,
                                  color_t color) {
  if ((x >= MULTI_DISPLAY_WIDTH) ||  // Clip right
      (y >= MULTI_DISPLAY_HEIGHT) || // Clip bottom
      ((x + 6 - 1) < 0) ||           // Clip left
      ((y + 8 - 1) < 0))             // Clip top
    return;

  for (int8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
    uint8_t line = pgm_read_byte(&standard_ascii_font[c * 5 + i]);
    for (int8_t j = 0; j < 8; j++, line >>= 1) {
      if (line & 1) {
        multi_display_draw_pixel(x + i, y + j, color);
      }
    }
  }
}

// Draw an ASCII character at (x, y) with specified color
// Convention: top left corner of screen is pixel (0, 0)
void stats_display_draw_character(int16_t x, int16_t y, unsigned char c,
                                  color_t color) {
  if ((x >= DISPLAY_WIDTH) ||  // Clip right
      (y >= DISPLAY_HEIGHT) || // Clip bottom
      ((x + 6 - 1) < 0) ||     // Clip left
      ((y + 8 - 1) < 0))       // Clip top
    return;

  for (int8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
    uint8_t line = pgm_read_byte(&standard_ascii_font[c * 5 + i]);
    for (int8_t j = 0; j < 8; j++, line >>= 1) {
      if (line & 1) {
        stats_display_draw_pixel(x + i, y + j, color);
      }
    }
  }
}

void multi_display_draw_character_size(int16_t x, int16_t y, unsigned char c, 
                                       color_t color, uint8_t size_x, uint8_t size_y) {

  if ((x >= MULTI_DISPLAY_WIDTH) ||   // Clip right
      (y >= MULTI_DISPLAY_HEIGHT) ||  // Clip bottom
      ((x + 6 * size_x - 1) < 0) ||     // Clip left
      ((y + 8 * size_y - 1) < 0)) {      // Clip top
    return;
  }

  for (int8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
    uint8_t line = pgm_read_byte(&standard_ascii_font[c * 5 + i]);
    for (int8_t j = 0; j < 8; j++, line >>= 1) {
      if (line & 1) {
        if (size_x == 1 && size_y == 1)
          multi_display_draw_pixel(x + i, y + j, color);
        else
          multi_display_draw_fill_rect(x + i * size_x, y + j * size_y, size_x, size_y, color);
      }
    }
  }
}

void stats_display_draw_character_size(int16_t x, int16_t y, unsigned char c, 
                                       color_t color, uint8_t size_x, uint8_t size_y) {

  if ((x >= DISPLAY_WIDTH) ||       // Clip right
      (y >= DISPLAY_HEIGHT) ||      // Clip bottom
      ((x + 6 * size_x - 1) < 0) || // Clip left
      ((y + 8 * size_y - 1) < 0)) { // Clip top
    return;
  }

  for (int8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
    uint8_t line = pgm_read_byte(&standard_ascii_font[c * 5 + i]);
    for (int8_t j = 0; j < 8; j++, line >>= 1) {
      if (line & 1) {
        if (size_x == 1 && size_y == 1)
          stats_display_draw_pixel(x + i, y + j, color);
        else
          stats_display_draw_fill_rect(x + i * size_x, y + j * size_y, size_x, size_y, color);
      }
    }
  }
}


void multi_display_draw_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, color_t color) {
  for (int16_t i = x; i < x + w; i++) {
    multi_display_draw_vertical_line(y, y + h, i, color);
  }
}

void stats_display_draw_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, color_t color) {
  for (int16_t i = x; i < x + w; i++) {
    stats_display_draw_vertical_line(y, y + h, i, color);
  }
}

void multi_display_configure_graph_axes(int16_t x_axis_min, int16_t x_axis_max,
                                        int16_t y_axis_min, int16_t y_axis_max) {

  graph_config.x_axis_min = x_axis_min;
  graph_config.x_axis_max = x_axis_max;
  graph_config.y_axis_min = y_axis_min;
  graph_config.y_axis_max = y_axis_max;

  multi_display_update_graph_configuration();
}

void multi_display_configure_graph_margins(uint8_t margin_left, uint8_t margin_right,
                                           uint8_t margin_top, uint8_t margin_bottom) {

  graph_config.margin_left = margin_left;
  graph_config.margin_right = margin_right;
  graph_config.margin_bottom = margin_bottom;
  graph_config.margin_top = margin_top;

  multi_display_update_graph_configuration();
}

void multi_display_update_graph_configuration(void) {
  // Update precomputed values for x_axis_span, y_axis_span,
  // y_horizontal, and x_vertical in the graph configuration struct
  graph_config.x_axis_span = graph_config.x_axis_max - graph_config.x_axis_min;
  graph_config.y_axis_span = graph_config.y_axis_max - graph_config.y_axis_min;

  graph_config.y_horizontal =
      graph_config.y_axis_max / graph_config.y_axis_span *
          (MULTI_DISPLAY_HEIGHT - graph_config.margin_top -
           graph_config.margin_bottom) +
      graph_config.margin_top;

  graph_config.x_vertical =
      -graph_config.x_axis_min / graph_config.x_axis_span *
          (MULTI_DISPLAY_WIDTH - graph_config.margin_left -
           graph_config.margin_right) +
      graph_config.margin_left;
}

void multi_display_draw_graph_axes(void) {
  // Draw horizontal line for the graph x-axis
  multi_display_draw_horizontal_line(
      graph_config.margin_left, MULTI_DISPLAY_WIDTH - graph_config.margin_right,
      graph_config.y_horizontal, COLOR_WHITE);

  // Draw vertical line for the graph y-axis
  multi_display_draw_vertical_line(graph_config.margin_top,
                                   MULTI_DISPLAY_HEIGHT -
                                       graph_config.margin_bottom,
                                   graph_config.x_vertical, COLOR_WHITE);

  // Draw label for x-axis maximum
  char x_buffer[6];
  snprintk(x_buffer, sizeof(x_buffer), "%d", graph_config.x_axis_max);

  for (size_t i = 1; i <= strlen(x_buffer); i++) {
    multi_display_draw_character(MULTI_DISPLAY_WIDTH -
                                     graph_config.margin_right - 5 * i,
                                 graph_config.y_horizontal + 1,
                                 x_buffer[strlen(x_buffer) - i], COLOR_WHITE);
  }

  // Draw label for y-axis maximum
  char y_buffer[6];
  snprintk(y_buffer, sizeof(y_buffer), "%d", graph_config.y_axis_max);

  for (size_t i = 1; i <= strlen(y_buffer); i++) {
    multi_display_draw_character(graph_config.x_vertical - 5 * i,
                                 graph_config.margin_top,
                                 y_buffer[strlen(y_buffer) - i], COLOR_WHITE);
  }
}

void multi_display_draw_graph_tick(uint32_t label) {
  // Draw horizontal tick for the graph x-axis
  multi_display_draw_horizontal_line(
      graph_config.margin_left - 3, graph_config.margin_left + 3,
      graph_config.y_horizontal, COLOR_WHITE);

  // Draw vertical line for the graph y-axis
  multi_display_draw_vertical_line(graph_config.margin_top,
                                   MULTI_DISPLAY_HEIGHT -
                                       graph_config.margin_bottom,
                                   graph_config.x_vertical, COLOR_WHITE);

  // Draw label for x-axis maximum
  char x_buffer[12];
  snprintk(x_buffer, sizeof(x_buffer), "%d", label);

  for (size_t i = 1; i <= strlen(x_buffer); i++) {
    multi_display_draw_character(MULTI_DISPLAY_WIDTH -
                                     graph_config.margin_right - 5 * i,
                                 graph_config.y_horizontal + 1,
                                 x_buffer[strlen(x_buffer) - i], COLOR_WHITE);
  }

  // Draw label for y-axis maximum
  char y_buffer[12];
  snprintk(y_buffer, sizeof(y_buffer), "%d", graph_config.y_axis_max);

  for (size_t i = 1; i <= strlen(y_buffer); i++) {
    multi_display_draw_character(graph_config.x_vertical - 5 * i,
                                 graph_config.margin_top,
                                 y_buffer[strlen(y_buffer) - i], COLOR_WHITE);
  }
}


void multi_display_draw_graph_data(float *x_values, float *y_values, uint16_t N, color_t color) {
  // Precompute scaling factors outside the loop
  float x_scale = (MULTI_DISPLAY_WIDTH - graph_config.margin_left - graph_config.margin_right) / graph_config.x_axis_span;
  float y_scale = (MULTI_DISPLAY_HEIGHT - graph_config.margin_top - graph_config.margin_bottom) / graph_config.y_axis_span;

  float x_offset = graph_config.margin_left - graph_config.x_axis_min * x_scale;
  float y_offset = graph_config.margin_top + graph_config.y_axis_max * y_scale;

  for (size_t i = 0; i < N; i++) {
    // Graph a single point
    float x = x_values[i] * x_scale + x_offset;
    float y = -y_values[i] * y_scale + y_offset;

    multi_display_draw_pixel(x, y, color);
  }
}

void stats_display_draw_data(float amplitude, float frequency) {
  stats_display_draw_character_size(5, 15, 'A', COLOR_WHITE, 2, 2);
  stats_display_draw_character_size(20, 15, '=', COLOR_WHITE, 2, 2);
  char a_buffer[12];
  snprintk(a_buffer, sizeof(a_buffer), "%f", amplitude);
  for (size_t i = 0; i < strlen(a_buffer); i++) {
    stats_display_draw_character_size(35 + 11 * i,
                                      15,
                                      a_buffer[i], 
                                      COLOR_WHITE,
                                      2,
                                      2);
  }
  
  stats_display_draw_character_size(5, 35, 'f', COLOR_WHITE, 2, 2);
  stats_display_draw_character_size(20, 35, '=', COLOR_WHITE, 2, 2);
  char f_buffer[12];
  snprintk(f_buffer, sizeof(f_buffer), "%f", frequency);
  for (size_t i = 0; i < strlen(f_buffer); i++) {
    stats_display_draw_character_size(35 + 11 * i,
                                      35,
                                      f_buffer[i], 
                                      COLOR_WHITE,
                                      2,
                                      2);
  }
}