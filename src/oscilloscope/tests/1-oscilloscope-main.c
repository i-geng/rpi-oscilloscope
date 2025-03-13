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
#include "nrf-test.h"
#include "test-interrupts.h"
#include "multi-display.h"
#include "multi-i2c.h"

// #define NRF_IRQ_PIN 22
#define NRF_IRQ_PIN 23
#define ADC_IRQ_PIN 17



nrf_t *nrf_client;
nrf_t *nrf_server;
ADC_STRUCT *adc;

void notmain(void) {
  kmalloc_init();

  uint32_t received_data_size = 1; //////////////
  float *received_data = kmalloc(sizeof(*received_data) * received_data_size);
  nrf_client = client_mk_noack(0xe5e5e5, sizeof(*received_data) * received_data_size); // RX address of the client 0xe5e5e5
  nrf_server = server_mk_noack(0, sizeof(fragment_t)); // RX address of the server 0

  // i2c_init();
  delay_ms(100);
  i2c_init_BSC0();
  delay_ms(100);
  i2c_init_BSC1();
  delay_ms(100);

  // Initialize displays
  multi_display_init();
  stats_display_init();
  multi_display_clear();
  stats_display_clear();

  multi_display_configure_graph_axes(0, samples, -3, 3);

  adc = adc_init(ADC_IRQ_PIN, PGA_6144, AIN0); // initialize ADC
  printk("[Main Pi] ADC initialized.\n");

  test_startup(NRF_IRQ_PIN, ADC_IRQ_PIN); // initialize interrupts

  for (int i = 0; i < 5000000; i ++){
    delay_ms(500);
  }
  // printk("[Main Pi] ADC data read.\n");

}