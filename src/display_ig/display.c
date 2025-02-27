#include "rpi.h"
#include "display.h"
#include "i2c.h"

static uint8_t buffer[1 + DISPLAY_BUFFER_SIZE];
static uint8_t *display_buffer = buffer + 1;

void display_init(void) {
  // Initialize I2C with some settling time
  delay_ms(100);
  i2c_init_clk_div(1500);
  delay_ms(100);

  // Display initialization flow [SSD1306 datasheet pg 64]
  // 0. Turn the display off to be safe [SSD1306 pg 28]
  display_send_command(0xAE);

  // 1. Set multiplex ratio
  display_send_command(0xA8);
  display_send_command(0x3F); // use offset 63 because display is 64 pixels tall

  // 2. Set display offset [SSD1306 pg 37]
  display_send_command(0xD3); // double byte command
  display_send_command(0x00); // map display start line to COM0 [SSD1306 pg 37]

  // 3. Set display start line [SSD1306 pg 36]
  display_send_command(0x40); // RAM row 0 is mapped to COM0

  // 4. Set segment re-map [SSD1306 pg 36]
  display_send_command(0xA0); // column addr 0 is mapped to SEG0

  // 5. Set COM output scan direction
  display_send_command(0xC8); // COM normal [SSD1306 pg 38]

  // 6. Set COM pins hardware configuration [SSD1306 pg 40]
  display_send_command(0xDA);
  display_send_command(0x12); // [SSD1206 pg 11] diagram says to use 0x12

  // 7. Set contrast control [SSD1306 pg 36]
  display_send_command(0x81); // double byte command
  // the chip has 256 contrast steps from 00h to FFH
  // recommended value seems to be 127 (middle of the range)
  display_send_command(0x7F);

  // 8. Display output according to GDDRAM contents [SSD1306 pg 37]
  display_send_command(0xA4);
  // display_send_command(0xA5);

  // 9. Set normal or inverse display [SSD1306 pg 37]
  display_send_command(0xA6); // normal
  // display_send_command(0xA7);  // inverse

  // 10. Set display clock divide ratio/oscillator frequency [SSD1306 pg 40]
  display_send_command(0xD5); // double byte command
  display_send_command(0x80); // default value ... formula on pg 22

  // 11. Enable charge pump regulator [SSD1306 pg 62]
  display_send_command(0x8D); // double byte command
  display_send_command(0x14); // enable charge pump

  // 12. Specify HORIZONTAL addressing mode [SSD1306 pg 35]
  // display_send_command(0x20);
  // display_send_command(0x02);   // page addressing mode [SSD1306 pg 34]
  display_send_command(0x20);
  display_send_command(0x00); // horizontal addressing mode
  display_send_command(0x21); // set column address
  display_send_command(0x00); // start at column 0
  display_send_command(0x7F); // end at column 127
  display_send_command(0x22); // set page address
  display_send_command(0x00); // start at page 0
  display_send_command(0x07); // end at page 7

  // 13. Display on [SSD1306 pg 62]
  display_send_command(0xAF);

  // 14. Clear the screen to black
  display_clear();
  display_show();
}

void display_send_command(uint8_t cmd) {
  uint8_t cmd_buf[2] = {0x00, cmd};
  i2c_write(DISPLAY_ADDRESS, cmd_buf, 2);
}

void display_show(void) {
  buffer[0] = 0x40; // control byte to indicate data
  i2c_write(DISPLAY_ADDRESS, buffer, sizeof(buffer));
}

void display_clear(void) {
  buffer[0] = 0x40; // control byte to indicate data
  memset(display_buffer, 0, DISPLAY_BUFFER_SIZE);
}

void display_fill_buffer(void) {
  buffer[0] = 0x40; // control byte to indicate data
  memset(display_buffer, 255, DISPLAY_BUFFER_SIZE);
}

void display_draw_pixel(uint16_t x, uint16_t y, color_t color) {
  // Convention: bottom left corner of screen is pixel (0, 0)

  switch (color) {
  case COLOR_WHITE:
    display_buffer[(y / 8) * DISPLAY_WIDTH + x] |= (1 << (y & 7));
    break;
  case COLOR_BLACK:
    display_buffer[(y / 8) * DISPLAY_WIDTH + x] &= ~(1 << (y & 7));
    break;
  case COLOR_INVERT:
    display_buffer[(y / 8) * DISPLAY_WIDTH + x] ^= (1 << (y & 7));
    break;
  }
}

void display_draw_axes(void) {}

// Draw a horizontal line from x_start to x_end (inclusive)
void draw_horizontal_line(int16_t x_start, int16_t x_end, int16_t y,
                          color_t color) {
  x_start = x_start < 0 ? 0 : x_start;
  x_end = x_end > DISPLAY_WIDTH - 1 ? DISPLAY_WIDTH - 1 : x_end;

  if ((y >= 0) && (y < DISPLAY_HEIGHT)) {
    int16_t w = x_end - x_start;
    uint8_t *pBuf = &display_buffer[(y / 8) * DISPLAY_WIDTH + x_start];
    uint8_t mask = 1 << (y & 7);

    switch (color) {
    case COLOR_WHITE:
      while (w--) {
        *pBuf++ |= mask;
      };
      break;
    case COLOR_BLACK:
      mask = ~mask;
      while (w--) {
        *pBuf++ &= mask;
      };
      break;
    case COLOR_INVERT:
      while (w--) {
        *pBuf++ ^= mask;
      };
      break;
    }
  }
}
