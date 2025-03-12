#ifndef __TEST_INTERRUPTS_H__
#define __TEST_INTERRUPTS_H__
// aggregate all the test definitions and code.
//  search for "TODO" below for the 6 routines to write.
#include "rpi.h"
#include "rpi-interrupts.h"
#include "rpi-inline-asm.h"
#include "cycle-count.h"

// we provide this code: does global initialization.  
//   see <test-interrupts.c>
typedef void (*init_fn_t)(void);

// client interrupt handler: returns 0 if didn't handle anything.
typedef int (*interrupt_fn_t)(uint32_t pc);
void test_startup(uint32_t NRF_IRQ_PIN, uint32_t ADC_IRQ_PIN);

// global state [bad form: but hopefully makes lab code more obvious]
extern volatile int n_interrupt;
// static uint32_t in_pin;
// enum { out_pin = 21, in_pin = 20 };
enum { N = 1024*32 };


#endif
