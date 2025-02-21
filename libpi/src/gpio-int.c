// engler, cs140 put your gpio-int implementations in here.
#include "rpi.h"

// in libpi/include: has useful enums.
#include "rpi-interrupts.h"

// [Broadcom pg 90]
enum {
  GPEDS0 = 0x20200040, // GPIO Pin Event Detect Status 0
  GPREN0 = 0x2020004C, // GPIO Pin Rising Edge Detect Enable 0
  GPFEN0 = 0x20200058, // GPIO Pin Falling Edge Detect Enable 0
};

static inline uint32_t OR32_DEV_BARRIER(uint32_t addr, uint32_t x) {
  dev_barrier();
  uint32_t v = GET32(addr);
  v |= x;
  PUT32(addr, v);
  dev_barrier();
  return v;
}
#define ptr_to_uint32(x) ((uint32_t)(ptrdiff_t)x)

static inline uint32_t or32_dev_barrier(volatile void *addr, uint32_t x) {
  return OR32_DEV_BARRIER(ptr_to_uint32(addr), x);
}

// returns 1 if there is currently a GPIO_INT0 interrupt,
// 0 otherwise.
//
// note: we can only get interrupts for <GPIO_INT0> since the
// (the other pins are inaccessible for external devices).
int gpio_has_interrupt(void) {
  // GPIO_INT[0] is general interrupt 49
  // We read bit 17 of IRQ_pending_2 register
  dev_barrier();
  uint32_t reg_val = GET32(IRQ_pending_2);
  dev_barrier();
  return (reg_val >> 17) & 0b1;
}

// gpio_int_rising_edge and gpio_int_falling_edge (and any other) should
// call this routine (you must implement) to setup the right GPIO event.
// as with setting up functions, you should bitwise-or in the value for the
// pin you are setting with the existing pin values.  (otherwise you will
// lose their configuration).  you also need to enable the right IRQ.   make
// sure to use device barriers!!
void enable_gpio_int(unsigned gpio_int) {
  uint32_t IRQ_Enable = IRQ_Enable_1 + 4 * (gpio_int / 32);
  uint32_t val = 0b1 << (gpio_int % 32);
  PUT32(IRQ_Enable, val);
  dev_barrier();
}

// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
void gpio_int_rising_edge(unsigned pin) {
  if (pin >= 32)
    return;

  // Set the [pin]-bit of GPREN0
  uint32_t val = 0b1 << pin;
  OR32_DEV_BARRIER(GPREN0, val);

  // Enable gpio_int[0], which is 49, since we are using pins less than 32
  enable_gpio_int(49);
}

// p98: detect falling edge (1->0).  sampled using the system clock.
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
void gpio_int_falling_edge(unsigned pin) {
  if (pin >= 32)
    return;

  // Set the [pin]-bit of GPFEN0
  uint32_t val = 0b1 << pin;
  OR32_DEV_BARRIER(GPFEN0, val);

  // Enable gpio_int[0], which is 49, since we are using pins less than 32
  enable_gpio_int(49);
}

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin) {
  if (pin >= 32)
    return 0;

  // Read the [pin]-bit of GPEDS0
  dev_barrier();
  uint32_t reg_val = GET32(GPEDS0);
  dev_barrier();
  return (reg_val >> pin) & 0b1;
}

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin) {
  if (pin >= 32)
    return;

  // Set the [pin]-bit of GPEDS0
  dev_barrier();
  uint32_t val = 0b1 << pin;
  PUT32(GPEDS0, val);
  dev_barrier();
}
