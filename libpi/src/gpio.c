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
    gpio_lev0  = (GPIO_BASE + 0x34)

    // <you may need other values.>
};

// set GPIO function for <pin> (input, output, alt...).  
// settings for other pins should be unchanged.
void gpio_set_function(unsigned pin, gpio_func_t function) {
  if (pin >= 32 && pin != 47) {
    return;
  }
  // [Broadcom pg 92 lists valid function values]
  if (function > 7) {
    return;
  }

  // Figure out which register address we need to access
  // [GPFSELn, Broadcom pg 90]
  uint32_t gpfsel_addr = GPIO_BASE + (pin / 10) * 4;

  // Read the value of the register
  uint32_t reg_val = GET32(gpfsel_addr);

  // Clear bit_first, (bit_first + 1), and (bit_first + 2)
  uint32_t bit_first = (pin % 10) * 3;
  uint32_t mask = ~(0b111 << bit_first);
  reg_val &= mask;

  // Write function bit pattern to correct 3 bits of reg_val
  reg_val |= (function << bit_first);

  // Write the new value back to the register
  PUT32(gpfsel_addr, reg_val);
}

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
  gpio_set_function(pin, 0b001);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
  if(pin >= 32 && pin != 47)
      return;

  // Figure out which register address we need to write to
  // [GPSETn, Broadcom pg 90]
  uint32_t gpset_addr = gpio_set0 + (pin / 32) * 4;

  // Write 0b1 to the correct bit to turn this pin on
  uint32_t value = 0b1 << (pin % 32);
  PUT32(gpset_addr, value);
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    if(pin >= 32 && pin != 47)
        return;

    // Figure out which register address we need to write to
    // [GPCLRn, Broadcom pg 90]
    uint32_t gpclr_addr = gpio_clr0 + (pin / 32) * 4;

    // Write 0b1 to the correct bit to turn this pin off
    uint32_t value = 0b1 << (pin % 32);
    PUT32(gpclr_addr, value);
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
  if (pin >= 32 && pin != 47) {
    return;
  }

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
  gpio_set_function(pin, 0b000);
}

// return the value of <pin>
int gpio_read(unsigned pin) {
  if (pin >= 32 && pin != 47) {
    return -1;
  }

  // Figure out which register address we need to access
  // [GPIO Pin Level Register, Broadcom pg 96]
  uint32_t gplev_addr = gpio_lev0 + (pin / 32 * 4);

  // Read register, shift it by N bits to the right, & it with 1
  unsigned v = GET32(gplev_addr);
  uint32_t N = pin % 32;
  v = v >> N;
  v &= 1;

  return DEV_VAL32(v);
}
