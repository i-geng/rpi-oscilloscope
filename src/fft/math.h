
// Define the resolution of sin and cos
#include "rpi.h"
#define RES 10
#define PI 3.14159265358979323846
#define TWO_PI (2 * PI)

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
