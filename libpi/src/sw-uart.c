#include "rpi.h"
#include "sw-uart.h"
#include "cycle-count.h"
#include "cycle-util.h"

#include <stdarg.h>

// bit bang the 8-bits <b> to the sw-uart <uart>.
//  - at 115200 baud you can probably just use microseconds,
//    but at faster rates you'd want to use cycles.
//  - libpi/include/cycle-util.h has some helper 
//    that you can use if you want (don't have to).
//
// recall: 
//    - the microseconds to write each bit (0 or 1) is in 
//      <uart->usec_per_bit>
//    - the cycles to write each bit (0 or 1) is in 
//      <uart->cycle_per_bit>
//    - <cycle_cnt_read()> counts cycles
//    - <timer_get_usec()> counts microseconds.
void sw_uart_put8(sw_uart_t *uart, uint8_t b) {
    // use local variables to minimize any loads or stores
    int tx = uart->tx;
    uint32_t T = uart->cycle_per_bit;
    uint32_t cyc_start = cycle_cnt_read();

    // Write a 0 (start) for T cycles
    // gpio_set_off(tx);
    // delay_ncycles(cyc_start, T);
    write_cyc_until(tx, 0, cyc_start, T);

    // Write each bit in the given value
    for (int i = 0; i < 8; i++) {
      // gpio_write(tx, b & 1);
      // gpio_set_on(tx);
      // delay_ncycles(cyc_start, T * (i + 2));
      // b = b >> 1;
      write_cyc_until(tx, b & 1, cyc_start, T * (i + 2));
      b = b >> 1;
    }

    // Write a 1 (stop) for at least T cycles
    gpio_set_on(tx);
    delay_ncycles(cyc_start, T * 10);
}

// optional: do receive.
//      EASY BUG: if you are reading input, but you do not get here in 
//      time it will disappear.
int sw_uart_get8_timeout(sw_uart_t *uart, uint32_t timeout_usec) {
    unsigned rx = uart->rx;

    // right away (used to be return never).
    while(!wait_until_usec(rx, 0, timeout_usec))
        return -1;

    // todo("implement this code\n");
    return 1;
}

// finish implementing this routine.  
sw_uart_t sw_uart_init_helper(unsigned tx, unsigned rx,
        unsigned baud, 
        unsigned cyc_per_bit,
        unsigned usec_per_bit) 
{
    // remedial sanity checking
    assert(tx && tx<31);
    assert(rx && rx<31);
    assert(cyc_per_bit && cyc_per_bit > usec_per_bit);
    assert(usec_per_bit);

    // basic sanity checking.  if this fails lmk
    unsigned mhz = 700 * 1000 * 1000;
    unsigned derived = cyc_per_bit * baud;
    if(! ((mhz - baud) <= derived && derived <= (mhz + baud)) )
        panic("too much diff: cyc_per_bit = %d * baud = %d\n", 
            cyc_per_bit, cyc_per_bit * baud);

    // Setup rx,tx and initial state of tx pin (high)
    gpio_set_output(tx);
    gpio_set_output(rx);
    gpio_set_on(tx);

    return (sw_uart_t) { 
            .tx = tx, 
            .rx = rx, 
            .baud = baud, 
            .cycle_per_bit = cyc_per_bit ,
            .usec_per_bit = usec_per_bit 
    };
}
