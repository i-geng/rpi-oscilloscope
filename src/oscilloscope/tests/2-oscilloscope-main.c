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

#define NRF_IRQ_PIN 23
#define ADC_IRQ_PIN 17

// 32-byte packaet fragment
typedef struct {
  uint8_t fragment; // 1 byte
  uint8_t total_fragments; // 1 byte
  uint8_t reserved[2]; // 2 bytes
  float data[7]; // 7 * 4 bytes = 28 bytes
} fragment_t;

void notmain(void) {
  kmalloc_init();

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

  multi_display_configure_graph_axes(0, 128, 0, 6);

  uint16_t base_samples = 128;
  uint16_t samples = base_samples;

  ADC_STRUCT *adc = adc_init(ADC_IRQ_PIN, PGA_6144, AIN0); // initialize ADC

  uint32_t received_data_size = 1;
  float *received_data = kmalloc(sizeof(*received_data) * received_data_size);
  nrf_t *nrf_client = client_mk_noack(0xe5e5e5, sizeof(*received_data) * received_data_size); // RX address of the client 0xe5e5e5
  nrf_t *nrf_server = server_mk_noack(0, sizeof(fragment_t)); // RX address of the server 0

  for (int i = 0; i < 5000000; i ++){
    float *data_array = kmalloc(sizeof(*data_array) * samples);

    for (int j = 0; j < samples; j ++){
      while(gpio_read(ADC_IRQ_PIN) == 0);
      data_array[j] = adc_read(adc);
    }

    float x_data[samples];
    for (int j = 0; j < samples; j ++){
      x_data[j] = j;
    }

    multi_display_clear();
    // multi_display_draw_graph_tick(i);
    multi_display_draw_graph_data(x_data, data_array, samples, COLOR_WHITE);
    multi_display_draw_graph_tick(samples * 1.2);
    // multi_display_draw_graph_axes();
    multi_display_show();  

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
      // printk("[Main Pi] Frequency: %f\n", frequency);
      stats_display_clear();
      stats_display_draw_data(112, frequency);
      stats_display_show();
    }
    else {
      // printk("[Main Pi] No data received from processing Pi\n");
    }
    delay_ms(500);
  }

}