#include "adc.h"
#include "i2c.h"

void notmain(){

    // init i2c
    i2c_init();

    int interrupt_pin = 17;
    ADC_STRUCT* adc = adc_init(interrupt_pin, PGA_6144);

    for (int j = 0; j < 1000; j ++){
        // printk("Attempting to read");
        float data = adc_read(adc);

        printk("Read %f from adc. \n", data);

        // Set scope sampling freq too
        delay_ms(0.625);
    }
}