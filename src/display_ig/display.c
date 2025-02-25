#include "rpi.h"
#include "i2c.h"
#include "display.h"

// static uint8_t *display_buffer = 0;
// static uint8_t display_buffer[DISPLAY_WIDTH * ((DISPLAY_HEIGHT + 7) / 8)];
static uint8_t display_buffer[128 * 8];

void display_init(void) {
  delay_ms(100);
  i2c_init_clk_div(1500);
  delay_ms(100);

  // Display initialization flow [SSD1306 datasheet pg 64]
  // 0. Turn the display off to be safe [SSD1306 pg 28]
  display_send_command(0xAE);

  // 1. Set multiplex ratio
  display_send_command(0xA8);
  display_send_command(0x3F);   // use offset 63 because display is 64 pixels tall

  // 2. Set display offset [SSD1306 pg 37]
  display_send_command(0xD3);   // double byte command
  display_send_command(0x00);   // map display start line to COM0 [SSD1306 pg 37]

  // 3. Set display start line [SSD1306 pg 36]
  display_send_command(0x40);   // RAM row 0 is mapped to COM0

  // 4. Set segment re-map [SSD1306 pg 36]
  display_send_command(0xA0);   // column addr 0 is mapped to SEG0

  // 5. Set COM output scan direction
  display_send_command(0xC8);   // COM normal [SSD1306 pg 38]

  // 6. Set COM pins hardware configuration [SSD1306 pg 40]
  display_send_command(0xDA);
  display_send_command(0x12);   // why 0x12?

  // 7. Set contrast control [SSD1306 pg 36]
  display_send_command(0x81);   // double byte command
  // the chip has 256 contrast steps from 00h to FFH
  // recommended value seems to be 127 (middle of the range)
  display_send_command(0x7F);

  // 8. Display output according to GDDRAM contents [SSD1306 pg 37]
  display_send_command(0xA4);
  // display_send_command(0xA5);

  // 9. Set normal or inverse display [SSD1306 pg 37]
  display_send_command(0xA6);   // normal
  // display_send_command(0xA7);  // inverse

  // 10. Set display clock divide ratio/oscillator frequency [SSD1306 pg 40]
  display_send_command(0xD5);   // double byte command
  display_send_command(0x80);   // default value ... formula on pg 22

  // 11. Enable charge pump regulator [SSD1306 pg 62]
  display_send_command(0x8D);   // double byte command
  display_send_command(0x14);   // enable charge pump

  // Specify horizontal addressing mode [SSD1306 pg 35]
  display_send_command(0x20);
  display_send_command(0x02);   // page addressing mode
    // // Set column address: from 0 to DISPLAY_WIDTH-1
    // display_send_command(0x21);         // Command for setting column address
    // display_send_command(0x00);         // Column start address (0)
    // // display_send_command(DISPLAY_WIDTH - 1); // Column end address
    // display_send_command(127);
  
    // // Set page address: from 0 to (DISPLAY_HEIGHT/8)-1
    // display_send_command(0x22);         // Command for setting page address
    // display_send_command(0x00);         // Page start address (0)
    // // display_send_command((DISPLAY_HEIGHT / 8) - 1); // Page end address
    // display_send_command(7);

  // 12. Display on [SSD1306 pg 62]
  display_send_command(0xAF);

  display_clear();
  display_fill_buffer();

  display_show();
}

void display_send_command(uint8_t cmd) {
  uint8_t buffer[2] = {0x00, cmd};
  i2c_write(DISPLAY_ADDRESS, buffer, 2);
}

void display_show(void) {
  for (uint8_t page = 0; page < 8; page++) {
    display_send_command(0xB0 + page); // Set page address
    display_send_command(0x00);        // Lower column start address
    display_send_command(0x10);        // Upper column start address

    // Send data for one page (128 bytes per page)
    uint8_t buffer[129]; // 1 byte prefix + 128 bytes data
    buffer[0] = 0x40; // Control byte for data stream
    memcpy(&buffer[1], &display_buffer[page * 128], 128);
    i2c_write(DISPLAY_ADDRESS, buffer, sizeof(buffer));
  }
  // int pages = DISPLAY_HEIGHT / 8;  // for 64: 8 pages

  // for (int page = 0; page < pages; page++) {
  //   // Set page address (0xB0 is the base for page addressing mode)
  //   display_send_command(0xB0 | page);
  //   // Set the column start address to 0 (split into two parts)
  //   display_send_command(0x00);  // lower nibble
  //   display_send_command(0x10);  // upper nibble

  //   // Calculate pointer to the current page in display_buffer.
  //   uint8_t *pageData = display_buffer + (page * DISPLAY_WIDTH);

  //   // Send the control byte (0x40) then the page data.
  //   // Since you don't want to allocate another buffer, we send the control byte separately.
  //   uint8_t ctrl = 0x40;
  //   i2c_write(DISPLAY_ADDRESS, &ctrl, 1);
  //   i2c_write(DISPLAY_ADDRESS, pageData, DISPLAY_WIDTH);
  // }
  
  // display_send_command(0x40);
  // uint8_t ctrl = 0x40;
  // i2c_write(DISPLAY_ADDRESS, &ctrl, 1);
  // i2c_write(DISPLAY_ADDRESS, display_buffer, sizeof(display_buffer));
}

void display_clear(void) {
  memset(display_buffer, 0, sizeof(display_buffer));
}

void display_fill_buffer(void) {
  memset(display_buffer, 255, sizeof(display_buffer));
}