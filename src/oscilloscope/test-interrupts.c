// test code for checking the interrupts. 
// see <test-interrupts.h> for additional routines.
#include "test-interrupts.h"
#include "timer-interrupt.h"
#include "vector-base.h"
#include "multi-display.h"
#include "multi-i2c.h"


float peak_to_peak = 0;
float frequency = 0;

static inline void gpio_clear_event_detect(unsigned pin) {
    // Write to clear event
    PUT32(0x20200000 + 0x40, (1 << pin));
    dev_barrier();
}

static float min(float *array, int size) {
    float min = array[0];
    for (int i = 1; i < size; i++) {
        if (array[i] < min) {
            min = array[i];
        }
    }
    return min;
}

static float max(float *array, int size) {
    float max = array[0];
    for (int i = 1; i < size; i++) {
        if (array[i] > max) {
            max = array[i];
        }
    }
    return max;
}

// default vector: just forwards it to the registered
// handler see <test-interrupts.h> and the given test.
void interrupt_vector(unsigned pc) {
    dev_barrier();
    static uint32_t adc_irq_count;
    // const uint32_t samples = 100;
    static float y_data[samples];
    static float x_data[samples];

    for (int i = 0; i < samples; i++) {
        x_data[i] = i;
    }

    // Check that the interrupt was caused by the gpio
    unsigned GPIO_gpeds0 = 0x20200000 + 0x40;
    unsigned event_status = GET32(GPIO_gpeds0);

    // Only proceed if interrupt was caused by gpio pin
    // printk("event status: %x\n", event_status);
    const uint32_t NRF_IRQ_PIN_2 = 23;
    const uint32_t ADC_IRQ_PIN = 17;

    if((event_status & (1 << NRF_IRQ_PIN_2))) {
        // printk("NRF interrupt pin %d\n", NRF_IRQ_PIN_2);
        gpio_clear_event_detect(NRF_IRQ_PIN_2);
        // nrf_clear();
        // draw text to third display();
        uint32_t received_data_size = 1; ///////////////////
        float received_data[received_data_size];

        if (nrf_read_exact_noblk(nrf_client, received_data, received_data_size * sizeof(*received_data)) == received_data_size * sizeof(*received_data)) {
            frequency = received_data[0];
            stats_display_clear();
            stats_display_draw_data(peak_to_peak, frequency);
            stats_display_show();
        }

    } 
    if(event_status & (1 << ADC_IRQ_PIN)) {
        // draw wave, send nrf data();
        // printk("ADC interrupt pin %d, reading ADC\n", ADC_IRQ_PIN);
        gpio_clear_event_detect(ADC_IRQ_PIN);
        float adc_read_data = (adc_read(adc) > 6) ? 0 : adc_read(adc);
        y_data[adc_irq_count % samples] = adc_read_data - 2.5;
        adc_irq_count++;
        if (adc_irq_count >= samples) {
            adc_irq_count = 0;

            // construct packet fragments and send each fragment one by one
            uint32_t total_fragments = samples / 7 + (samples % 7 != 0);

            for (int j = 0; j < total_fragments; j ++) {
                fragment_t packet;
                packet.fragment = j;
                packet.total_fragments = total_fragments;
                packet.packet_size = samples;
                for (int k = 0; k < 7; k ++) {
                    if (j * 7 + k < samples) {
                        packet.data[k] = y_data[j * 7 + k];
                    } else {
                        packet.data[k] = -99; // -99 padding
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
            

            float min_y = min(y_data, samples);
            float max_y = max(y_data, samples);
            peak_to_peak = max_y - min_y;

            stats_display_clear();
            stats_display_draw_data(peak_to_peak, frequency);
            stats_display_show();

            disable_interrupts();
            multi_display_clear();
            // multi_display_draw_graph_tick(i);
            multi_display_draw_graph_data(x_data, y_data, samples, COLOR_WHITE);
            multi_display_draw_graph_tick(samples * 1.2);
            // multi_display_draw_graph_axes();
            multi_display_show();  

            enable_interrupts();

        }
        
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