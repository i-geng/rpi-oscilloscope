#ifndef __TEST_INTERRUPTS_H__
#define __TEST_INTERRUPTS_H__
// aggregate all the test definitions and code.
//  search for "TODO" below for the 6 routines to write.
#include "rpi.h"
#include "rpi-interrupts.h"
#include "rpi-inline-asm.h"
#include "cycle-count.h"
#include "nrf.h"
#include "adc.h"

extern nrf_t *nrf_client;
extern nrf_t *nrf_server;
extern ADC_STRUCT *adc;

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


// 32-byte packaet fragment
typedef struct {
  uint8_t fragment; // 1 byte
  uint8_t total_fragments; // 1 byte
  // uint8_t reserved[2]; // 2 bytes
  uint16_t packet_size; // 2 bytes
  float data[7]; // 7 * 4 bytes = 28 bytes
} fragment_t;



#endif
