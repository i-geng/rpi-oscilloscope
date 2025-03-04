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
  int pga_gain;
  float pga_val;
} ADC_STRUCT;

typedef enum {
  PGA_6144 = 0b000,
  PGA_4096 = 0b001,
  PGA_2048 = 0b010,
  PGA_1024 = 0b011,
  PGA_0512 = 0b100,
  PGA_0256 = 0b111
} ADC_GAIN;


// Initialize adc
ADC_STRUCT* adc_init(int interrupt_pin, ADC_GAIN gain);

// Read from adc
float adc_read(ADC_STRUCT* adc);
