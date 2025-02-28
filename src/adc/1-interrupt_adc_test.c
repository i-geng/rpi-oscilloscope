#include "adc.h"


// Test the adc with alert/interrupt pin
// Uses while loop to wait for interrupt pin to go high
void notmain(){

    int interrupt_pin = 17;
    ADC_STRUCT* adc = adc_init(interrupt_pin);

    uint32_t start_time = timer_get_usec_raw(); // Start timer
    uint32_t last_time = start_time;
    for (int j = 0; j < 1000; j ++){
        // printk("Attempting to read");
        while(gpio_read(interrupt_pin) == 0);

        uint32_t current_time = timer_get_usec_raw(); 
        uint16_t data = adc_read(adc);

        uint32_t time_since_last = current_time - last_time;
        last_time = current_time;

        // printk("Read %d from adc. \n", data);
        if (j % 100 == 0)
            printk("time: %d.%d ms, delta: %d.%d ms, data: %d \n", 
            (int) ((current_time - start_time) / 1000), 
            (int) ((current_time - start_time) % 1000),
            (int) (time_since_last / 1000), 
            (int) (time_since_last % 1000), 
            (int) data);
    }
    printk("1000 samples done. Time taken: %d us \n", (int) timer_get_usec_raw() - start_time);
    printk("Average time per sample: %d us \n", (int) (timer_get_usec_raw() - start_time) / 1000);
}