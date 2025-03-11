/*
This test draws the raw input from signal generator to one display.
The other display will show the amplitutde and frequency. 
A potentiometer is used to control the horizontal scaling of the graph.
The ADC data is sent to another Pi over NRF for signal processing.
*/

/*
Main Pi RX address: 0xe5e5e5
Processing (cloud) Pi RX address: 0xe6e6e6
*/

#include "rpi.h"
#include "i2c.h"
#include "adc.h"
#include "multi-display.h"
#include "nrf-test.h"
// #include "gpio.h"

// #define NRF_IRQ_PIN 22
#define NRF_IRQ_PIN 23
#define ADC_IRQ_PIN 17

// from interrupts-asm.S
void disable_interrupts(void);
void enable_interrupts(void);

enum {
  IRQ_Base            = 0x2000b200,
  IRQ_basic_pending   = IRQ_Base+0x00,    // 0x200
  IRQ_pending_1       = IRQ_Base+0x04,    // 0x204
  IRQ_pending_2       = IRQ_Base+0x08,    // 0x208
  IRQ_FIQ_control     = IRQ_Base+0x0c,    // 0x20c
  IRQ_Enable_1        = IRQ_Base+0x10,    // 0x210
  IRQ_Enable_2        = IRQ_Base+0x14,    // 0x214
  IRQ_Enable_Basic    = IRQ_Base+0x18,    // 0x218
  IRQ_Disable_1       = IRQ_Base+0x1c,    // 0x21c
  IRQ_Disable_2       = IRQ_Base+0x20,    // 0x220
  IRQ_Disable_Basic   = IRQ_Base+0x24,    // 0x224
};

enum {
  GPIO_Base           = 0x20200000,
  GPIO_fsel0          = GPIO_Base,
  GPIO_fsel1          = GPIO_Base + 0x04,
  GPIO_fsel2          = GPIO_Base + 0x08,
  GPIO_fsel3          = GPIO_Base + 0x0C,
  GPIO_gpeds0         = GPIO_Base + 0x40,
  GPIO_gpren0         = GPIO_Base + 0x4C 
};

#define GPIO_IRQ_0_31    (1 << 17)

// We assume you already have GET32/PUT32, dev_barrier, etc.
extern unsigned GET32(unsigned);
extern void PUT32(unsigned, unsigned);
extern void dev_barrier(void);

// We'll store ADC samples here
volatile float adc_data[1000];
volatile int sample_count = 0;
volatile uint32_t start_time, last_time;

// 32-byte packaet fragment
typedef struct {
  uint8_t fragment; // 1 byte
  uint8_t total_fragments; // 1 byte
  uint8_t reserved[2]; // 2 bytes
  float data[7]; // 7 * 4 bytes = 28 bytes
} fragment_t;


static inline void gpio_enable_rising_edge_detect(unsigned pin) {
  // RMW on GPREN0 (only have 32 pins)
  unsigned reg_val = GET32(GPIO_gpren0);
  reg_val |= (1 << pin);
  PUT32(GPIO_gpren0, reg_val);

  dev_barrier();
}

// Clear event detect status for pin
static inline void gpio_clear_event_detect(unsigned pin) {
  // Write to clear event
  PUT32(GPIO_gpeds0, (1 << pin));
  dev_barrier();
}

// This function enables the GPIO 0–31 interrupt in the interrupt controller.
static void enable_gpio_int_0_31(void) {
  unsigned val = GET32(IRQ_Enable_2);
  val |= GPIO_IRQ_0_31;  // enable interrupt line for GPIO pins 0–31
  PUT32(IRQ_Enable_2, val);
  dev_barrier();
}

// Basic interrupt init: copy vector table, disable all interrupts, etc.
// Often your environment already does something like this — adapt as needed.
static void interrupt_init(void) {
  disable_interrupts();

  // Disable all interrupt sources first
  PUT32(IRQ_Disable_1, 0xFFFFFFFF);
  PUT32(IRQ_Disable_2, 0xFFFFFFFF);
  dev_barrier();

  // Copy the interrupt vector table to 0 if not already done
  extern unsigned _interrupt_table[];
  extern unsigned _interrupt_table_end[];

  unsigned *dst = (unsigned*)0;
  unsigned n = _interrupt_table_end - _interrupt_table;

  gcc_mb();
  for(unsigned i = 0; i < n; i++)
    dst[i] = _interrupt_table[i];
  gcc_mb();

  dev_barrier();
}

// void interrupt_vector(unsigned pc) {
//     dev_barrier();

//     // Check that the interrupt was caused by the gpio
//     unsigned event_status = GET32(GPIO_gpeds0);

//     // Only proceed if interrupt was caused by gpio pin
//     if(event_status & (1 << interrupt_pin)) {

//         // Clear the event detect so we don't loop forever
//         gpio_clear_event_detect(interrupt_pin);
//         // Call our ADC read logic
//         // (this used to be "gpio_interrupt_handler" function)
        
//         uint32_t current_time = timer_get_usec_raw();
//         uint32_t time_since_last = current_time - last_time;
//         last_time = current_time;

//         // Read ADC data
//         adc_data[sample_count] = adc_read(adc);

//         // Print every 100 samples
//         // if ((sample_count % 10) == 0) {
//         //     printk("time: %d.%d ms, delta: %d.%d ms, data: %d\n", 
//         //            (int)((current_time - start_time) / 1000),
//         //            (int)((current_time - start_time) % 1000),
//         //            (int)(time_since_last / 1000),
//         //            (int)(time_since_last % 1000),
//         //            adc_data[sample_count]);
//         // }
//         // printk("Read ")

//         sample_count++;

//         // // Stop collecting after 1000 samples
//         // if (sample_count >= 1000) {
//         //     unsigned end_time = timer_get_usec_raw();
//         //     unsigned elapsed  = end_time - start_time;
//         //     printk("1000 samples done. Time taken: %d us\n", elapsed);
//         //     printk("Average time per sample: %d us\n", elapsed / 1000);

//         //     // Usually we *wouldn't* disable interrupts, but 
//         //     // for a simple test, we can:
//         //     disable_interrupts();
//         // }
//     }
//     dev_barrier();
// }

static void enable_nrf_interrupt(void) {
  gpio_set_input(NRF_IRQ_PIN);
  gpio_set_pullup(NRF_IRQ_PIN);
  gpio_enable_rising_edge_detect(NRF_IRQ_PIN);
  
  // Enable the GPIO interrupt
  enable_gpio_int_0_31();
}


void notmain(void) {
  kmalloc_init();

  uint16_t base_samples = 100;
  uint16_t samples = base_samples;

  uint32_t received_data_size = 1;
  float *received_data = kmalloc(sizeof(*received_data) * received_data_size);
  nrf_t *nrf_client = client_mk_noack(0xe5e5e5, sizeof(*received_data) * received_data_size); // RX address of the client 0xe5e5e5
  nrf_t *nrf_server = server_mk_noack(0, sizeof(fragment_t)); // RX address of the server 0

  enable_interrupts();
  for (int i = 0; i < 5000000; i ++){
    float *data_array = kmalloc(sizeof(*data_array) * samples);

    for (int j = 0; j < samples; j ++){
      data_array[j] = j;
    }
    // construct packet fragments and send each fragment one by one
    uint32_t total_fragments = samples / 7 + (samples % 7 != 0);

    for (int j = 0; j < total_fragments; j ++) {
      fragment_t packet;
      packet.fragment = j;
      packet.total_fragments = total_fragments;
      for (int k = 0; k < 7; k ++) {
        if (j * 7 + k < samples) {
          packet.data[k] = data_array[j * 7 + k];
        } else {
          packet.data[k] = -1;
        }
      }
      nrf_tx_send_noack(nrf_server, 0xe6e6e6, &packet, sizeof(packet)); // send data to address 0xe6e6e6 (RX address of processing Pi)
      printk("[Main Pi] Sent fragment #%d of %d.\n", packet.fragment, packet.total_fragments);
      printk("[Main Pi] Data: ");
      for (int k = 0; k < 7; k ++) {
        printk("%f ", packet.data[k]);
      }
      printk("\n");
    }
    printk("[Main Pi] Sent all fragments.\n");

    if (nrf_read_exact_noblk(nrf_client, received_data, received_data_size * sizeof(*received_data)) == received_data_size * sizeof(*received_data)) {
      float frequency = received_data[0];
      printk("[Main Pi] Frequency: %f\n", frequency);
    }
    else {
      // printk("[Main Pi] No data received from processing Pi\n");
    }
    delay_ms(500);
  }

}