#include "math.h"

float normalize_angle(float x) {
    return fmod(x + PI, TWO_PI) - PI;
}

float fmod(float x, float div) {
    // normalize x
	//if div or x is nan,return x directly
	if (div != div || x != x) {
		return x;
	}
	if (div == 0) {
		return 0;
	}
	return x - (int) (x / div) * div;
}
float sin(float x)
{
    x = normalize_angle(x);
    float res=0, pow=x, fact=1;
    for(int i=0; i<RES; ++i)
    {
        res+=pow/fact;
        pow*=-1*x*x;
        fact*=(2*(i+1))*(2*(i+1)+1);
    }

    return res;
}
float cos(float x)
{
    x = normalize_angle(x);
    float res=1, pow=1, fact=1;  // Note: cos starts with 1, not 0
    x = x * x;  // Pre-square x since cos uses even powers
    
    for(int i=1; i<RES; ++i)
    {
        pow *= -1 * x;  // Alternating sign, x^2 each time
        fact *= (2*i-1) * (2*i);  // Factorial terms: 2, 4, 6, 8...
        res += pow/fact;
    }
    return res;
}

void plot_signal(
    float *signal, 
    int length, 
    float shift,
    float scale
) {
    for (int i = 0; i < length; i++) {
        int stars = (int)((signal[i] + shift) * scale + 0.5f);
        for (int j = 0; j < stars; j++) {
            printk("*");
        }
        printk("\n");
    }
}