#include "rpi.h"
#include "i2c.h"
#include "display.h"

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
  display_send_command(0x12);   // [SSD1206 pg 11] diagram says to use 0x12

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

  // 12. Specify PAGE addressing mode [SSD1306 pg 34] (I don't think our chip has other modes)
  // display_send_command(0x20);
  // display_send_command(0x02);   // page addressing mode
  display_send_command(0x20);
  display_send_command(0x00);   // horizontal addressing mode
    display_send_command(0x21);
    display_send_command(0x00);
    display_send_command(127);
    display_send_command(0x22);
    display_send_command(0x00);
    display_send_command(0x7);

  // 13. Display on [SSD1306 pg 62]
  display_send_command(0xAF);

  display_clear();
  display_fill_buffer();

  display_show();
}

void display_send_command(uint8_t cmd) {
  uint8_t cmd_buf[2] = {0x00, cmd};
  i2c_write(DISPLAY_ADDRESS, cmd_buf, 2);
}

void display_show(void) {
  buffer[0] = 0x40;   // control byte to indicate data
  i2c_write(DISPLAY_ADDRESS, buffer, sizeof(buffer));
}

void display_clear(void) {
  buffer[0] = 0x40;   // control byte to indicate data
  memset(display_buffer, 0, DISPLAY_BUFFER_SIZE);
}

void display_fill_buffer(void) {
  buffer[0] = 0x40;   // control byte to indicate data
  memset(display_buffer, 255, DISPLAY_BUFFER_SIZE);
}