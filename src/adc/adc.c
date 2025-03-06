// ASSUMING THE ADDRESS BYTE IS WRITTEN BY THE I2C HARDWARE

#include "i2c.h"
#include "rpi.h"
#include "adc.h"
// #include "stdlib.h"

// #define ADC_ADDR 0x48

void adc_write_to_reg(ADC_STRUCT* adc, int reg, uint16_t bytes_2){
  uint8_t buf[3];
  
  adc->pointed_reg = reg;

  buf[0] = (uint8_t) reg;
  buf[1] = bytes_2 >> 8;
  buf[2] = bytes_2 & 0xFF;

  int status = i2c_write(adc->addr, buf, 3);
  assert(status);
}

void adc_point_to_reg(ADC_STRUCT* adc, int reg){
  uint8_t buf[1];
  
  adc->pointed_reg = reg;

  buf[0] = (uint8_t) reg;

  int status = i2c_write(adc->addr, buf, 1);
  assert(status);
}

uint16_t adc_read_reg(ADC_STRUCT* adc, int reg){
  uint8_t buf[2];
  
  // Check that we are pointing to the register
  // we're trying to read from, else die
  assert(adc->pointed_reg == reg);

  int status = i2c_read(adc->addr, buf, 2);
  assert(status);

  return (uint16_t) (buf[0] << 8)|buf[1];
}

float adc_read(ADC_STRUCT* adc){
  int16_t data = adc_read_reg(adc, CONVERSION_REG);

  return (data / (32768.0)) * adc->pga_val;
}


ADC_STRUCT* adc_init(uint8_t addr, int interrupt_pin, ADC_GAIN gain){

  // kmalloc_init();
  ADC_STRUCT* adc = (ADC_STRUCT*) kmalloc(sizeof(ADC_STRUCT));

  adc->pga_gain = gain;
  adc->addr = addr;
  switch(gain){
    case PGA_6144: adc->pga_val = 6.144; break;
    case PGA_4096: adc->pga_val = 4.096; break;
    case PGA_2048: adc->pga_val = 2.048; break;
    case PGA_1024: adc->pga_val = 1.024; break;
    case PGA_0512: adc->pga_val = 0.512; break;
    case PGA_0256: adc->pga_val = 0.256; break;
  }

  uint16_t config = 0;
  
  config |= 0b1   << 14;  // Don't need to set this
  config |= 0b100 << 12;  // Set pos input to AIN0, nev input to GND
  config |= gain <<  9;  // TODO: Set PGA correctly
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


  // Setting up the CE pin
  adc->interrupt_pin = interrupt_pin;
  gpio_set_function(adc->interrupt_pin, GPIO_FUNC_INPUT);
  gpio_set_pulldown(adc->interrupt_pin);

  return adc;
}
