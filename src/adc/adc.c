// ASSUMING THE ADDRESS BYTE IS WRITTEN BY THE I2C HARDWARE

#include "i2c.h"
#include "rpi.h"

#define ADC_ADDR 0x48

enum ADC_REG {
  CONVERSION_REG, 
  CONFIG_REG,
  LO_THRESH_REG,
  HI_THRESH_REG
};

typedef struct {
  int pointed_reg;
} ADC_STRUCT;

void adc_write_to_reg(ADC_STRUCT* adc, int reg, uint16_t bytes_2){
  uint8_t buf[3];
  
  adc->pointed_reg = reg;

  buf[0] = (uint8_t) reg;
  buf[1] = bytes_2 >> 8;
  buf[2] = bytes_2 & 0xFF;

  int status = i2c_write(ADC_ADDR, buf, 3);
  assert(status);
}

void adc_point_to_reg(ADC_STRUCT* adc, int reg){
  uint8_t buf[1];
  
  adc->pointed_reg = reg;

  buf[0] = (uint8_t) reg;

  int status = i2c_write(ADC_ADDR, buf, 1);
  assert(status);
}

uint16_t adc_read_reg(ADC_STRUCT* adc, int reg){
  uint8_t buf[2];
  
  // Check that we are pointing to the register
  // we're trying to read from, else die
  assert(adc->pointed_reg == reg);

  int status = i2c_read(ADC_ADDR, buf, 2);
  assert(status);

  return (uint16_t) (buf[0] << 8)|buf[1];
}

uint16_t adc_read(ADC_STRUCT* adc){
  return adc_read_reg(adc, CONVERSION_REG);
}


ADC_STRUCT* adc_init(void) {

  kmalloc_init();
  ADC_STRUCT* adc = (ADC_STRUCT*) kmalloc(sizeof(ADC_STRUCT));

  uint16_t config = 0;
  
  config |= 0b1   << 14;  // Don't need to set this
  config |= 0b100 << 12;  // Set pos input to AIN0, nev input to GND
  config |= 0b000 <<  9;  // TODO: Set PGA correctly
  config |= 0b0   <<  8;  // Set to continuous mode
  config |= 0b111 <<  5;  // 860 SPS data rate.
  // don't care 4
  config |= 0b1   <<  3;  // Set Interrupt pin to active HI
  // don't care 2
  config |= 0b00  <<  0;  // Assert interrupt after 1 conversion

  // Configure device
  adc_write_to_reg(adc, CONFIG_REG, config);
  
  // Enable interrupts on valid
  uint16_t hi_thresh = 1 << 15;
  uint16_t lo_thresh = 1 <<  0;
  adc_write_to_reg(adc, HI_THRESH_REG, hi_thresh);
  adc_write_to_reg(adc, LO_THRESH_REG, lo_thresh);

  // Point to digital output register to prepare for reads
  adc_point_to_reg(adc, CONVERSION_REG);

  return adc;
}
