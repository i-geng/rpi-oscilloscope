#include "adc.h"
#include "i2c.h"

void notmain(){

    i2c_init();
    ADC_STRUCT* adc = adc_init();

    for (int j = 0; j < 1000; j ++){
        // printk("Attempting to read");
        uint16_t data = adc_read(adc);

        // printk("Read %d from adc. \n", data);
        printk("%d \n", (int) data);

        // Set scope sampling freq too
        delay_ms(0.625);
    }
}