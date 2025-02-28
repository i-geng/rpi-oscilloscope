#include "display.h"
#include "my_i2c.h"
#include "i2c.h"
#include "rpi.h"

void notmain(void) {

    delay_ms(100);
    i2c_init_clk_div(1500);
    delay_ms(100);

    display_init();
    display_show();

    display_draw_character(40, 40, '1', COLOR_WHITE);
    display_draw_character(44, 40, '2', COLOR_WHITE);

    display_draw_graph_axes();

    // Must call display_show() to actually update the screen
    display_show();

    while (1) {}
}