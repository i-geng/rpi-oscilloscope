#include "rpi.h"
// #include "display.h"
#include "i2c.h"
#include "adc.h"
#include "multi-display.h"

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

  i2c_init();

  // Initialize display
  multi_display_init();
  ADC_STRUCT* adc = adc_init(17, PGA_6144);
  float data;
  for (int i = 0; i < 50000; i ++){
    data = adc_read(adc);
    printk("Read %f from adc. \n", data);
  
    int int_data = (int) (1000*data);
    convert_data_to_ascii(int_data);
    multi_display_show();
    // delay_ms(1000);
    multi_display_clear();

  }
}