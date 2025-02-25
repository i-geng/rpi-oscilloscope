#include "rpi.h"
#include "display.h"

void notmain(void) {
  display_init();

  while (1) {}
}