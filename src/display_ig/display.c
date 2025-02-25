#include "rpi.h"
#include "i2c.h"
#include "display.h"

// static uint8_t *display_buffer = 0;
static uint8_t display_buffer[DISPLAY_WIDTH * ((DISPLAY_HEIGHT + 7) / 8)];

void display_init(void) {
  delay_ms(100);
  i2c_init_clk_div(1500);
  delay_ms(100);

  // Send SSD1306 initialization commands
  display_send_command(0xAE); // Display OFF
  display_send_command(0xD5); // Set display clock divide ratio
  display_send_command(0x80);
  display_send_command(0xA8); // Set multiplex ratio
  display_send_command(0x3F);
  display_send_command(0xD3); // Set display offset
  display_send_command(0x00);
  display_send_command(0x40); // Set display start line
  display_send_command(0x8D); // Charge pump setting
  display_send_command(0x14);
  display_send_command(0x20); // Memory addressing mode
  display_send_command(0x00);
  display_send_command(0xA1); // Segment remap
  display_send_command(0xC8); // COM output scan direction
  display_send_command(0xDA); // COM pins hardware configuration
  display_send_command(0x12);
  display_send_command(0x81); // Contrast control
  display_send_command(0xCF);
  display_send_command(0xD9); // Pre-charge period
  display_send_command(0xF1);
  display_send_command(0xDB); // VCOMH deselect level
  display_send_command(0x40);
  display_send_command(0xA4); // Entire display ON
  display_send_command(0xA6); // Normal display mode
  display_send_command(0xAF); // Display ON

  display_clear();
  display_fill_buffer();

  display_show();
}

void display_send_command(uint8_t cmd) {
  uint8_t buffer[2] = {0x00, cmd};
  i2c_write(DISPLAY_ADDRESS, buffer, 2);
}

void display_show(void) {
  // Set column address: from 0 to DISPLAY_WIDTH-1
  display_send_command(0x21);         // Command for setting column address
  display_send_command(0x00);         // Column start address (0)
  display_send_command(DISPLAY_WIDTH - 1); // Column end address

  // Set page address: from 0 to (DISPLAY_HEIGHT/8)-1
  display_send_command(0x22);         // Command for setting page address
  display_send_command(0x00);         // Page start address (0)
  display_send_command((DISPLAY_HEIGHT / 8) - 1); // Page end address

  display_send_command(0x40);
  i2c_write(DISPLAY_ADDRESS, display_buffer, sizeof(display_buffer));
}

void display_clear(void) {
  memset(display_buffer, 0, sizeof(display_buffer));
}

void display_fill_buffer(void) {
  memset(display_buffer, 255, sizeof(display_buffer));
}