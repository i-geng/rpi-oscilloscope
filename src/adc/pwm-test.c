#include "rpi.h"

void notmain()
{
    gpio_set_output(19);
    while(1)
    {
        gpio_set_on(19);
        delay_ms(15);
        gpio_set_off(19);
        delay_ms(15);
    }

}