// Define the resolution of sin and cos
#include "rpi.h"
#define RES 3
#define PI 3.14159265358979323846
#define TWO_PI (2 * PI)
#define HALF_PI (PI / 2)

#define USE_FAST_MATH 1

float normalize_angle(float x);
float fmod(float x, float div);
float sin(float x);
float cos(float x);

void plot_signal(
    float *signal, 
    int length, 
    float shift,
    float scale
);

// FFT function declarations
void fft(float data_re[], float data_im[], const unsigned int N);
void rearrange(float data_re[], float data_im[], const unsigned int N);
void compute(float data_re[], float data_im[], const unsigned int N);

float sqrt(float x);
