// test code for checking the interrupts. 
// see <test-interrupts.h> for additional routines.
#include "test-interrupts.h"
#include "timer-interrupt.h"
#include "vector-base.h"

static inline void gpio_clear_event_detect(unsigned pin) {
    // Write to clear event
    PUT32(0x20200000 + 0x40, (1 << pin));
    dev_barrier();
}


// default vector: just forwards it to the registered
// handler see <test-interrupts.h> and the given test.
void interrupt_vector(unsigned pc) {
    dev_barrier();

    // Check that the interrupt was caused by the gpio
    unsigned GPIO_gpeds0 = 0x20200000 + 0x40;
    unsigned event_status = GET32(GPIO_gpeds0);

    // Only proceed if interrupt was caused by gpio pin
    // printk("event status: %x\n", event_status);
    const uint32_t NRF_IRQ_PIN_2 = 23;
    const uint32_t ADC_IRQ_PIN = 17;

    if((event_status & (1 << NRF_IRQ_PIN_2))) {
        // draw text to third display();
        printk("NRF interrupt pin %d\n", NRF_IRQ_PIN_2);
        gpio_clear_event_detect(NRF_IRQ_PIN_2);
        // nrf_clear();/
    } 
    if(event_status & (1 << ADC_IRQ_PIN)) {
        // draw wave, send nrf data();
        printk("ADC interrupt pin %d\n", ADC_IRQ_PIN);
        gpio_clear_event_detect(ADC_IRQ_PIN);

    }
    dev_barrier();
}

/********************************************************************
 * falling edge.
 */

volatile int n_falling;

// check if there is an event, check if it was a falling edge.
int falling_handler(uint32_t in_pin) {
    // todo("implement this: return 0 if no rising int\n");
    if(gpio_event_detected(in_pin)) {
        if(gpio_read(in_pin) == 0)
        {
            n_falling++;
            gpio_event_clear(in_pin);
            return 1;
        }
    }
    return 0;
}

/********************************************************************
 * rising edge.
 */

volatile int n_rising;

// check if there is an event, check if it was a rising edge.
int rising_handler(uint32_t in_pin) {
    // todo("implement this: return 0 if no rising int\n");
    if(gpio_event_detected(in_pin)) {
        if(gpio_read(in_pin) == 1)
        {
            n_rising++;
            gpio_event_clear(in_pin);
            return 1;
        }
    }
    return 0;
}

void test_startup(uint32_t NRF_IRQ_PIN, uint32_t ADC_IRQ_PIN) {
    extern uint32_t interrupt_vec[];
    int_vec_init(interrupt_vec);

    gpio_set_input(NRF_IRQ_PIN);
    gpio_set_input(ADC_IRQ_PIN);

    gpio_int_falling_edge(NRF_IRQ_PIN);
    gpio_int_rising_edge(ADC_IRQ_PIN);

    // interrupt_fn = int_fn;
    falling_handler(NRF_IRQ_PIN);
    rising_handler(ADC_IRQ_PIN);

    // in case there was an event queued up clear it.
    gpio_event_clear(NRF_IRQ_PIN);
    gpio_event_clear(ADC_IRQ_PIN);

    // start global interrupts.
    cpsr_int_enable();
}