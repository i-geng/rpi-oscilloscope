#ifndef __RPI_DISPLAY_H__
#define __RPI_DISPLAY_H__

#define DISPLAY_ADDRESS 0X3C
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * ((DISPLAY_HEIGHT + 7) / 8))

typedef enum {
  COLOR_WHITE,
  COLOR_BLACK,
  COLOR_INVERT,
} color_t;

void display_init(void);

void display_send_command(uint8_t cmd);

void display_show(void);

void display_draw_pixel(uint16_t x, uint16_t y, color_t color);

void display_clear(void);

void display_fill_buffer(void);

#endif