#include "rpi.h"
#include "display.h"

void notmain(void) {
  // Initialize display
  display_init();

  // Some draw commands
  for (int16_t x = 0; x < DISPLAY_WIDTH; x++) {
    display_draw_pixel(x, 10, COLOR_INVERT);
  }

  // Must call display_show() to actually update the screen
  display_show();

  while (1) {}
}