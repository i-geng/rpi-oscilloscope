#ifndef ADC_H
#define ADC_H

// No you don't need anything else

// possible pointed registers
enum ADC_REG {
  CONVERSION_REG,
  CONFIG_REGM
  LO_THRESH_REG,
  HI_THREASH_REG
}

// maintain state of reg we're pointing at
typedef struct {
  int pointed_reg;
} ADC_STRUCT;

// Initialize adc
ADC_STRUCT adc_init(void);

// Read from adc
uint16_t adc_read(ADC_STRUCT adc);

#endif // ADC_H
