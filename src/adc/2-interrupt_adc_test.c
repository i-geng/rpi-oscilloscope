#include "rpi.h"
#include "adc.h"

//-------------------------------------------------------------------
// 1) GLOBALS AND DEFINES
//-------------------------------------------------------------------

#define GPIO_BASE        0x20200000   // BCM2835, Pi Zero–3
#define IRQ_BASE         0x2000B200
#define GPIO_GPREN0      (GPIO_BASE + 0x4C)  // Rising Edge Detect
#define GPIO_GPEDS0      (GPIO_BASE + 0x40)  // Event Detect Status
#define IRQ_Enable_2     (IRQ_BASE + 0x14)

// For GPIO 0-31 on Pi Zero–3, interrupt is bit 17 in IRQ_Enable_2
#define GPIO_IRQ_0_31    (1 << 17)

// We'll use GPIO pin 17 as our "interrupt_pin"
static const int interrupt_pin = 17;

// We assume you already have GET32/PUT32, dev_barrier, etc.
extern unsigned GET32(unsigned);
extern void PUT32(unsigned, unsigned);
extern void dev_barrier(void);

// from interrupts-asm.S
void disable_interrupts(void);
void enable_interrupts(void);

// We'll store ADC samples here
volatile uint16_t adc_data[1000];
volatile int sample_count = 0;
volatile uint32_t start_time, last_time;

// Global pointer to ADC struct
ADC_STRUCT *adc = 0;

//-------------------------------------------------------------------
// 2) LOW-LEVEL GPIO AND INTERRUPT SETUP
//-------------------------------------------------------------------

// Enable rising-edge detect for pin. (No external function needed.)
static inline void gpio_enable_rising_edge_detect(unsigned pin) {
    unsigned reg_val = GET32(GPIO_GPREN0);
    reg_val |= (1 << pin);
    PUT32(GPIO_GPREN0, reg_val);
    dev_barrier();
}

// Clear event detect status for pin
static inline void gpio_clear_event_detect(unsigned pin) {
    // Writing '1' to bit <pin> clears it
    PUT32(GPIO_GPEDS0, (1 << pin));
    dev_barrier();
}

// This function enables the GPIO 0–31 interrupt in the interrupt controller.
static void enable_gpio_int_0_31(void) {
    unsigned val = GET32(IRQ_Enable_2);
    val |= GPIO_IRQ_0_31;  // enable interrupt line for GPIO pins 0–31
    PUT32(IRQ_Enable_2, val);
    dev_barrier();
}

// Basic interrupt init: copy vector table, disable all interrupts, etc.
// Often your environment already does something like this — adapt as needed.
static void interrupt_init(void) {
    disable_interrupts();

    // 1) Disable all IRQ sources so we start from a known state
    #define IRQ_Disable_1 (IRQ_BASE + 0x1c)
    #define IRQ_Disable_2 (IRQ_BASE + 0x20)
    PUT32(IRQ_Disable_1, 0xFFFFFFFF);
    PUT32(IRQ_Disable_2, 0xFFFFFFFF);
    dev_barrier();

    // 2) Copy the interrupt vector table to 0 if not already done
    extern unsigned _interrupt_table[];
    extern unsigned _interrupt_table_end[];
    unsigned *dst = (unsigned*)0;
    unsigned n = _interrupt_table_end - _interrupt_table;
    for(unsigned i = 0; i < n; i++)
        dst[i] = _interrupt_table[i];
    dev_barrier();
}

//-------------------------------------------------------------------
// 3) GLOBAL INTERRUPT HANDLER
//-------------------------------------------------------------------
// This is called from interrupts-asm.S if an IRQ occurs.
void interrupt_vector(unsigned pc) {
    dev_barrier();

    // 1) Check if our GPIO pin triggered an event
    unsigned event_status = GET32(GPIO_GPEDS0);
    if(event_status & (1 << interrupt_pin)) {
        // Clear the event detect so we don't loop forever
        gpio_clear_event_detect(interrupt_pin);
        // Call our ADC read logic
        // (this used to be "gpio_interrupt_handler" function)
        
        uint32_t current_time = timer_get_usec_raw();
        uint32_t time_since_last = current_time - last_time;
        last_time = current_time;

        // Read ADC data
        adc_data[sample_count] = adc_read(adc);

        // Print every 100 samples
        if ((sample_count % 100) == 0) {
            printk("time: %d.%03d ms, delta: %d.%03d ms, data: %d\n", 
                   (int)((current_time - start_time) / 1000),
                   (int)((current_time - start_time) % 1000),
                   (int)(time_since_last / 1000),
                   (int)(time_since_last % 1000),
                   adc_data[sample_count]);
        }

        sample_count++;

        // Stop collecting after 1000 samples
        if (sample_count >= 1000) {
            unsigned end_time = timer_get_usec_raw();
            unsigned elapsed  = end_time - start_time;
            printk("1000 samples done. Time taken: %d us\n", elapsed);
            printk("Average time per sample: %d us\n", elapsed / 1000);

            // Usually we *wouldn't* disable interrupts, but 
            // for a simple test, we can:
            disable_interrupts();
        }
    }

    dev_barrier();
}

//-------------------------------------------------------------------
// 4) MAIN CODE
//-------------------------------------------------------------------
void notmain(void) {
    printk("Initializing interrupts...\n");
    interrupt_init();   // set up system-level IRQ state

    printk("Configuring GPIO pin %d as input, with pull-up...\n", interrupt_pin);
    gpio_set_input(interrupt_pin);
    gpio_set_pullup(interrupt_pin);

    printk("Enabling rising-edge detect...\n");
    gpio_enable_rising_edge_detect(interrupt_pin);

    printk("Enabling GPIO 0–31 interrupt line in the interrupt controller...\n");
    enable_gpio_int_0_31();

    printk("Initializing ADC...\n");
    adc = adc_init(interrupt_pin);

    // Reset sampling state
    sample_count = 0;
    start_time   = timer_get_usec_raw();
    last_time    = start_time;

    printk("Enabling global interrupts...\n");
    enable_interrupts();

    printk("Waiting for 1000 samples...\n");
    while(sample_count < 1000) {
        // In a real program, do other work here
        // or go to sleep. The interrupt will handle ADC sampling.
    }

    printk("Data collection complete.\n");
    // optionally do more post-processing...
    // done!
    return;
}
