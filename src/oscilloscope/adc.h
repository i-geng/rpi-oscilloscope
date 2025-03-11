#ifndef ADC_H
#define ADC_H
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
  uint8_t addr;
  int pointed_reg;
  int interrupt_pin;
  int pga_gain;
  float pga_val;
  uint32_t config;
} ADC_STRUCT;

typedef enum {
  PGA_6144 = 0b000,
  PGA_4096 = 0b001,
  PGA_2048 = 0b010,
  PGA_1024 = 0b011,
  PGA_0512 = 0b100,
  PGA_0256 = 0b111
} ADC_GAIN;

typedef enum {
  AIN0 = 0b100,
  AIN1 = 0b101,
  AIN2 = 0b110,
  AIN3 = 0b111
} ADC_CHANNEL;

// swap input channel
void adc_change_channel(ADC_STRUCT* adc, ADC_CHANNEL new_channel);

// Initialize adc
ADC_STRUCT* adc_init(int interrupt_pin, ADC_GAIN gain, ADC_CHANNEL channel);

// Read from adc
float adc_read(ADC_STRUCT* adc);

#endif