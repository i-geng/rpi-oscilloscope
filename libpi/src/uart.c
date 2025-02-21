// simple mini-uart driver: implement every routine 
// with a <todo>.
//
// NOTE: 
//  - from broadcom: if you are writing to different 
//    devices you MUST use a dev_barrier().   
//  - its not always clear when X and Y are different
//    devices.
//  - pay attenton for errata!   there are some serious
//    ones here.  if you have a week free you'd learn 
//    alot figuring out what these are (esp hard given
//    the lack of printing) but you'd learn alot, and
//    definitely have new-found respect to the pioneers
//    that worked out the bcm eratta.
//
// historically a problem with writing UART code for
// this class (and for human history) is that when 
// things go wrong you can't print since doing so uses
// uart.  thus, debugging is very old school circa
// 1950s, which modern brains arne't built for out of
// the box.   you have two options:
//  1. think hard.  we recommend this.
//  2. use the included bit-banging sw uart routine
//     to print.   this makes things much easier.
//     but if you do make sure you delete it at the 
//     end, otherwise your GPIO will be in a bad state.
//
// in either case, in the next part of the lab you'll
// implement bit-banged UART yourself.
#include "rpi.h"

// change "1" to "0" if you want to comment out
// the entire block.
#if 1
//*****************************************************
// We provide a bit-banged version of UART for debugging
// your UART code.  delete when done!
//
// NOTE: if you call <emergency_printk>, it takes 
// over the UART GPIO pins (14,15). Thus, your UART 
// GPIO initialization will get destroyed.  Do not 
// forget!   

// header in <libpi/include/sw-uart.h>
#include "sw-uart.h"
static sw_uart_t sw_uart;

// if we've ever called emergency_printk better
// die before returning.
static int called_sw_uart_p = 0;

// a sw-uart putc implementation.
static int sw_uart_putc(int chr) {
    sw_uart_put8(&sw_uart,chr);
    return chr;
}

// call this routine to print stuff. 
//
// note the function pointer hack: after you call it 
// once can call the regular printk etc.
static void emergency_printk(const char *fmt, ...) {
    // track if we ever called it.
    called_sw_uart_p = 1;


    // we forcibly initialize each time it got called
    // in case the GPIO got reset.
    // setup gpio 14,15 for sw-uart.
    sw_uart = sw_uart_default();

    // all libpi output is via a <putc>
    // function pointer: this installs ours
    // instead of the default
    rpi_putchar_set(sw_uart_putc);

    printk("NOTE: HW UART GPIO is in a bad state now\n");

    // do print
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);

}

#undef todo
#define todo(msg) do {                      \
    emergency_printk("%s:%d:%s\nDONE!!!\n",      \
            __FUNCTION__,__LINE__,msg);   \
    rpi_reboot();                           \
} while(0)

// END of the bit bang code.
#endif


//*****************************************************
// the rest you should implement.

enum {
  AUXENB = 0x20215004,          // [Broadcom pg 9]
  AUX_MU_IO_REG = 0x20215040,   // [Broadcom pg 11]
  AUX_MU_IER_REG = 0x20215044,  // [Broadcom pg 12]
  AUX_MU_IIR_REG = 0x20215048,  // [Broadcom pg 13]
  AUX_MU_LCR_REG = 0x2021504C,  // [Broadcom pg 14]
  AUX_MU_MCR_REG = 0x20215050,  // [Broadcom pg 14]
  AUX_MU_LSR_REG = 0x20215054,  // [Broadcom pg 15]
  AUX_MU_CNTL_REG = 0x20215060, // [Broadcom pg 16]
  AUX_MU_STAT_REG = 0x20215064, // [Broadcom pg 18]
  AUX_MU_BAUD = 0x20215068,     // [Broadcom pg 19]
};

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
  // NOTE: make sure you delete all print calls when
  // done!
  // emergency_printk("start here\n");

  // perhaps confusingly: at this point normal printk works
  // since we overrode the system putc routine.
  // printk("write UART addresses in order\n");

  dev_barrier();

  /* Configure the GPIO 14 and 15 pins to use ALT5 function [Broadcom pg 102] */
  gpio_set_function(GPIO_TX, GPIO_FUNC_ALT5);
  gpio_set_function(GPIO_RX, GPIO_FUNC_ALT5);

  dev_barrier();

  /* Enable UART [Broadcom pg 9] */
  uint32_t reg_val = GET32(AUXENB);
  // Set bit0
  PUT32(AUXENB, reg_val | 0b1);

  dev_barrier();

  /* Disable TX and RX [Broadcom pg 17] */
  // Clear bit0 and bit1 to disable the transmitter and receiver
  PUT32(AUX_MU_CNTL_REG, 0b00);

  /* Disable TX and RX interrupts [Broadcom pg 12] */
  // Clear bit0 and bit1 to disable TX and RX interrupts
  PUT32(AUX_MU_IER_REG, 0b00);

  /* Clear the TX and RX FIFO queues [Broadcom pg 13] */
  // Set bit1 and bit2 to clear the transmit and receive queues
  PUT32(AUX_MU_IIR_REG, 0b110);

  /* Configure UART to work in 8-bit mode [Broadcom pg 14] */
  // Set bits0:1
  PUT32(AUX_MU_LCR_REG, 0b11);

  /* Why is this here? */
  PUT32(AUX_MU_MCR_REG, 0);

  /* Configure 115200 Baud [Broadcom pg 19] */
  // Write 270 to bits0:15 (formula on Broadcom pg 11)
  PUT32(AUX_MU_BAUD, 270);

  /* Enable TX and RX [Broadcom pg 17] */
  // Set bit0 and bit1
  PUT32(AUX_MU_CNTL_REG, 0b11);

  dev_barrier();

  // delete everything to do w/ sw-uart when done since
  // it trashes your hardware state and the system
  // <putc>.
  demand(!called_sw_uart_p, 
      delete all sw-uart uses or hw UART in bad state);
}

// disable the uart: make sure all bytes have been
// 
void uart_disable(void) {
  dev_barrier();

  uart_flush_tx();

  // Read the value of the AUXENB register [Broadcom pg 9]
  uint32_t reg_val = GET32(AUXENB);

  // Clear bit 0
  reg_val &= 0b0;

  // Write the new value back to the AUXENB register
  PUT32(AUXENB, reg_val);

  dev_barrier();
}

// returns one byte from the RX (input) hardware
// FIFO.  if FIFO is empty, blocks until there is 
// at least one byte.
int uart_get8(void) {
  dev_barrier();

  // Block until the FIFO has a byte
  while (uart_has_data() == 0) {}

  // [Broadcom pg 11]
  uint32_t reg_val = GET32(AUX_MU_IO_REG);
  int data = reg_val & 0xff;

  dev_barrier();
  return data;
}

// returns 1 if the hardware TX (output) FIFO has room
// for at least one byte.  returns 0 otherwise.
int uart_can_put8(void) {
  // If bit5 is set, then the TX FIFO has room [Broadcom pg 18]
  uint32_t reg_val = GET32(AUX_MU_LSR_REG);
  uint32_t can_put = reg_val >> 5;
  can_put &= 0b1;

  return can_put;
}

// put one byte on the TX FIFO, if necessary, waits
// until the FIFO has space.
int uart_put8(uint8_t c) {
  dev_barrier();

  // Block until the TX FIFO has space
  while (uart_can_put8() == 0) {}

  // When writing, only bits0:7 are taken [Broadcom pg 11]
  PUT32(AUX_MU_IO_REG, c);

  dev_barrier();
  return c;
}

// returns:
//  - 1 if at least one byte on the hardware RX FIFO.
//  - 0 otherwise
int uart_has_data(void) {
  // [Broadcom pg 18]
  uint32_t reg_val = GET32(AUX_MU_LSR_REG);
  int has_data = reg_val & 0b1;
  return has_data;
}

// returns:
//  -1 if no data on the RX FIFO.
//  otherwise reads a byte and returns it.
int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

// returns:
//  - 1 if TX FIFO empty AND idle.
//  - 0 if not empty.
int uart_tx_is_empty(void) {
  dev_barrier();
  
  // [Broadcom pg 15]
  uint32_t reg_val = GET32(AUX_MU_LSR_REG);
  int tx_is_empty = reg_val & 0x40;

  dev_barrier();
  return tx_is_empty;
}

// return only when the TX FIFO is empty AND the
// TX transmitter is idle.  
//
// used when rebooting or turning off the UART to
// make sure that any output has been completely 
// transmitted.  otherwise can get truncated 
// if reboot happens before all bytes have been
// received.
void uart_flush_tx(void) {
  while(!uart_tx_is_empty())
      rpi_wait();
}
