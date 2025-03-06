#include "rpi.h"
#include "i2c.h"
#include "adc.h"
#include "multi-display.h"
#include "signal_processing.h"

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


  // uint16_t samples = 50;
  uint16_t base_samples = 200;
  // uint16_t upsamples = 1000;

  i2c_init();

  // Initialize display
  multi_display_init();
  printk("Display initialized. \n");

  ADC_STRUCT* adc = adc_init(0x48, 17, PGA_6144);

  ADC_STRUCT* adc2 = adc_init(0x49, 17, PGA_6144);




  for (int i = 0; i < 5000000; i ++){
    // float *data_array = kmalloc(sizeof(*data_array) * samples);
    // for (int j = 0; j < samples; j ++){
    //   data = adc_read(adc);
    //   data_array[j] = data > 0 ? data : 0;
    // }


    while(gpio_read(17) == 0);
    int horizontal_scaling = (int) (adc_read(adc2) * 10);
    uint16_t samples = base_samples/2 * (horizontal_scaling + 1);
    multi_display_configure_graph_axes(0, samples, 0, 3.3);

    float *data_array = kmalloc(sizeof(*data_array) * samples);

    float data;
    float graph_index[samples];
    for (int i = 0; i < samples; i ++){
      graph_index[i] = i;
    }


    // printk("%d \n", horizontal_scaling);

    for (int j = 0; j < samples; j ++){
      data = adc_read(adc);
      data_array[j] = data > 0 ? data : 0;
    }

    // float *data_array = generate_dummy_signal(samples, 1);
    

    // Do the thing
    // float* out = ENHANCE(data_array, samples, upsamples);

    // for (int j = 0; j < 100; j ++){
    //   printk("%f \n", out[j]);
    // }
    // printk("\n");

    multi_display_clear();
    multi_display_draw_graph_data(graph_index, data_array, (uint16_t)samples, COLOR_WHITE);
    multi_display_show();
  }

}