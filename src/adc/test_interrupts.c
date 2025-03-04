// test code for checking the interrupts. 
// see <test-interrupts.h> for additional routines.
#include "test-interrupts.h"
#include "timer-interrupt.h"
#include "vector-base.h"

volatile int n_interrupt;

static interrupt_fn_t interrupt_fn;

// default vector: just forwards it to the registered
// handler see <test-interrupts.h> and the given test.
void interrupt_vector(unsigned pc) {
    dev_barrier();
    n_interrupt++;

    if(!interrupt_fn(pc))
        panic("should have no other interrupts?\n");

    dev_barrier();
}

// initialize all the interrupt stuff.  client passes in the 
// gpio int routine <fn>
//
// make sure you understand how this works.
void test_startup(init_fn_t init_fn, interrupt_fn_t int_fn) {
    output("\tImportant: must loop back (attach a jumper to) pins 20 & 21\n");
    output("\tImportant: must loop back (attach a jumper to) pins 20 & 21\n");
    output("\tImportant: must loop back (attach a jumper to) pins 20 & 21\n");

    // initialize.
    extern uint32_t interrupt_vec[];
    int_vec_init(interrupt_vec);

    gpio_set_output(out_pin);
    gpio_set_input(in_pin);

    init_fn();
    interrupt_fn = int_fn;

    // in case there was an event queued up clear it.
    gpio_event_clear(in_pin);

    // start global interrupts.
    cpsr_int_enable();
}


/********************************************************************
 * falling edge.
 */

volatile int n_falling;

// check if there is an event, check if it was a falling edge.
int falling_handler(uint32_t pc) {
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

void falling_init(void) {
    gpio_write(out_pin, 1);
    gpio_int_falling_edge(in_pin);
}

/********************************************************************
 * rising edge.
 */

volatile int n_rising;

// check if there is an event, check if it was a rising edge.
int rising_handler(uint32_t pc) {
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

void rising_init(void) {
    gpio_write(out_pin, 0);
    gpio_int_rising_edge(in_pin);
}

/********************************************************************
 * timer interrupt
 */

void timer_test_init(void) {
    // turn on timer interrupts.
    timer_init(1, 0x4);
}

// make sure this gets called on each timer interrupt.
int timer_test_handler(uint32_t pc) {
    dev_barrier();
    // should look very similar to the timer interrupt handler.
    // todo("implement this by stealing pieces from 4-interrupts/0-timer-int");

    // dev_barrier();

    // get the interrupt source: typically if you have 
    // one interrupt enabled, you'll have > 1, so have
    // to disambiguate what the source was.
    unsigned pending = GET32(IRQ_basic_pending);

    if((pending & ARM_Timer_IRQ) == 0)
        return 0;

    PUT32(ARM_Timer_IRQ_Clear, 1);

    // note: <staff-src/timer.c:timer_get_usec_raw()> 
    // accesses the timer device, which is different 
    // than the interrupt subsystem.  so we need
    // a dev_barrier() before.
    // dev_barrier();

    n_interrupt++;

    // // compute time since the last interrupt.
    // static unsigned last_clk = 0;
    // unsigned clk = timer_get_usec_raw();
    // static volatile unsigned period, period_sum;
    // period = last_clk ? clk - last_clk : 0;
    // period_sum += period;
    // last_clk = clk;

    // we don't know what the client was doing, 
    // so, again, do a barrier at the end of the
    // interrupt handler.
    //
    // NOTE: i think you can make an argument that 
    // this barrier is superflous given that the timer
    // access is a read that we wait for, but for the 
    // moment we live in simplicity: there's enough 
    // bad stuff that can happen with interrupts that
    // we don't need to do tempt entropy by getting cute.
    dev_barrier();  
    return 1;
}
