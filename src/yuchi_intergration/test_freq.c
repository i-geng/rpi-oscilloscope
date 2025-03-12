#include "rpi.h"
#include "i2c.h"
#include "adc.h"
#include "multi-display.h"

#define ADC_INTERRUPT_PIN 17

int get_interval_usec(
    uint32_t start_time,
    uint32_t end_time
) {
    if (end_time < start_time) {
        // Wrap around
        return (0xFFFFFFFF - start_time) + end_time;
    }
    return end_time - start_time;
}


void notmain(void) {

    kmalloc_init();

    i2c_init();
    multi_display_init();

    // Initialize ADC
    ADC_STRUCT* adc = adc_init(ADC_INTERRUPT_PIN, PGA_6144, AIN0);

    unsigned int count = 0;
   

    unsigned int test_duration = 1; // seconds

    // Allocate a buffer to store time stamps (test_duration * 1000 samples)
    uint32_t* time_stamps = kmalloc(test_duration * 1000 * sizeof(uint32_t));

    uint32_t start_time = timer_get_usec();
    uint32_t current_time = start_time;
    while (
        get_interval_usec(start_time, current_time) < test_duration * 1000000
    ) {
        while(gpio_read(ADC_INTERRUPT_PIN) == 0);
        float voltage = adc_read(adc);
        // // Get current time stamp
        current_time = timer_get_usec();
        time_stamps[count] = current_time;
        count++;

        // Test update the display
        multi_display_clear();
        // multi_display_draw_graph_axes();
        multi_display_show();
    }

    printk("Sample rate: %d Hz\n", count / test_duration);
    // Calculate the average interval between samples
    uint32_t average_interval = 0;
    for (unsigned int i = 0; i < count - 1; i++) {
        uint32_t interval = get_interval_usec(time_stamps[i], time_stamps[i+1]);
        average_interval += interval;
    }
    average_interval /= (count - 1);
    printk("Average interval: %d us\n", average_interval);

    // Calculate the standard deviation of the intervals
    uint32_t standard_deviation = 0;
    for (unsigned int i = 0; i < count - 1; i++) {
        standard_deviation += (average_interval - get_interval_usec(time_stamps[i], time_stamps[i+1])) * (average_interval - get_interval_usec(time_stamps[i], time_stamps[i+1]));
    }
    standard_deviation /= (count - 1);
    printk("Standard deviation: %d us\n", standard_deviation);
    return;
}