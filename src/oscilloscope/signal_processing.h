#ifndef __SIGNAL_PROCESSING_H__
#define __SIGNAL_PROCESSING_H__

#include <complex.h>
#include "rpi.h"
#include "adc.h"

#define pi           3.14159265358979323846

float* generate_dummy_signal(uint16_t samples,  uint16_t freq);

float* ENHANCE(float* signal, uint16_t N, uint16_t M);

#endif