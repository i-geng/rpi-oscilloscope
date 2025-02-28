#include "rpi.h"

// No you don't need anything else

// possible pointed registers
typedef enum {
  CONVERSION_REG,
  CONFIG_REG,
  LO_THRESH_REG,
  HI_THRESH_REG
} ADC_REG;

// maintain state of reg we're pointing at
typedef struct {
  int pointed_reg;
  int interrupt_pin;
} ADC_STRUCT;

// Initialize adc
ADC_STRUCT* adc_init(int interrupt_pin);

// Read from adc
uint16_t adc_read(ADC_STRUCT* adc);
