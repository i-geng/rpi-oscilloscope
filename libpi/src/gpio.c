/*
 * Implement the following routines to set GPIO pins to input or 
 * output, and to read (input) and write (output) them.
 *  - DO NOT USE loads and stores directly: only use GET32 and 
 *    PUT32 to read and write memory.  
 *  - DO USE the minimum number of such calls.
 * (Both of these matter for the next lab.)
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"

// see broadcomm documents for magic addresses.
//
// if you pass addresses as:
//  - pointers use put32/get32.
//  - integers: use PUT32/GET32.
//  semantics are the same.
enum {
    GPIO_BASE = 0x20200000,
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34),

    // <you may need other values.>
};

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!

void gpio_set_function(unsigned pin, gpio_func_t func) {
  if(pin >= 32 && pin != 47)
        return;
  if (func > 7)
    return;
  // implement this
  // use <gpio_fsel0>
  int reg_id = pin / 10;
  int reg_offset = pin % 10;
  unsigned value = GET32(GPIO_BASE + reg_id * 4);
  int shift = reg_offset * 3;
  int bits = func;
  unsigned mask = 7 << shift;

  value &= ~mask;
  value |= bits << shift;

  PUT32(GPIO_BASE + reg_id * 4, value);
}


void gpio_set_output(unsigned pin) {
  gpio_set_function(pin, 1);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    if(pin >= 32 && pin != 47)
        return;
  // implement this
  // use <gpio_set0>
  int reg_id = pin / 32;
  int reg_offset = pin % 32;
  PUT32(gpio_set0 + reg_id * 4, 1 << reg_offset);
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    if(pin >= 32 && pin != 47)
        return;
  // implement this
  // use <gpio_clr0>
  int reg_id = pin / 32;
  int reg_offset = pin % 32;
  PUT32(gpio_clr0 + reg_id * 4, 1 << reg_offset);
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
  // implement.
  gpio_set_function(pin, 0);
}

// return the value of <pin>
int gpio_read(unsigned pin) {
  if(pin >= 32 && pin != 47)
    return -1;
  unsigned v = 0;

  unsigned addr = gpio_lev0 + (pin / 32) * 4;
  unsigned value = GET32(addr);
  int shift = pin % 32;
  v = (value >> shift) & 1;

  return DEV_VAL32(v);
}