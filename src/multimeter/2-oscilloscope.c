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
#include "signal_processing.h"
#include "nrf-test.h"

void convert_data_to_ascii(int data) {
  char buffer[10];
  snprintk(buffer, sizeof(buffer), "%d  ", data);

  for (size_t i = 1; i <= strlen(buffer); i++) {
    multi_display_draw_character(50 - 5 * i,
                                 32,
                                 buffer[strlen(buffer) - i], COLOR_WHITE);

    multi_display_draw_character(170 - 5 * i,
                                 32,
                                 buffer[strlen(buffer) - i], COLOR_WHITE);
  }
}

void notmain(void) {
  kmalloc_init();
  i2c_init();
  multi_display_init();
  printk("Display initialized. \n");

  ADC_STRUCT* adc = adc_init(0x48, 17, PGA_6144); // for reading signal
  ADC_STRUCT* adc2 = adc_init(0x49, 17, PGA_6144); // for potentiometer

  uint16_t base_samples = 200;

  for (int i = 0; i < 5000000; i ++){
    while(gpio_read(17) == 0);

    int horizontal_scaling = (int) (adc_read(adc2) * 10); // 0-50 input from potentiometer
    uint16_t samples = base_samples/2 * (horizontal_scaling + 1);

    multi_display_configure_graph_axes(0, samples, 0, 3.3);
    float *data_array = kmalloc(sizeof(*data_array) * samples);

    float data;
    float graph_index[samples];
    for (int i = 0; i < samples; i ++){
      graph_index[i] = i;
    }

    for (int j = 0; j < samples; j ++){
      data = adc_read(adc);
      data_array[j] = data > 0 ? data : 0;
    }

    unsigned nbytes = sizeof(*data_array) * samples;
    nrf_t *nrf_server = server_mk_noack(0, nbytes); // RX address of the server 0
    nrf_t *nrf_client = client_mk_noack(0xe5e5e5, nbytes); // RX address of the client 0xe5e5e5
    // nrf_stat_start(nrf_server);

    nrf_tx_send_noack(nrf_server, 0xe6e6e6, &data_array, nbytes); // send data_array to address 0xe6e6e6 (RX address of processing Pi)

    uint32_t received_data_size = 1; // frequency
    float *received_data = kmalloc(sizeof(*received_data) * received_data_size);
    if (nrf_read_exact_noblk(nrf_server, received_data, received_data_size * sizeof(*received_data)) == received_data_size * sizeof(*received_data)) {
      float frequency = received_data[0];
      printk("Frequency: %f\n", frequency);
    }

    multi_display_clear();
    multi_display_draw_graph_data(graph_index, data_array, (uint16_t)samples, COLOR_WHITE);
    multi_display_show();
  }

}