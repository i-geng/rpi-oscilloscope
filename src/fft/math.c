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


float faster_cos(float x) {
    float x2 = x * x;
    return 1 + (x2 / 2.0f) * (-1 + (x2/12.0f) * (1 + (x2/30.0f) * (-1 + (x2/56.0f) )));
}

float normal_cos(float x) {
    float res=1, pow=1, fact=1;
    for(int i=1; i<RES; ++i)
    {
        pow *= -1 * x;
        fact *= (2*i-1) * (2*i);
        res += pow/fact;
    }
    return res;
}



float cos(float x)
{
    x = normalize_angle(x);
#if USE_FAST_MATH
    return faster_cos(x);
#else
    return normal_cos(x);
#endif
}

float faster_sin(float x) {
    float x2 = x * x;
    return x * (1 + (x2 / 6.0f) * (-1 + (x2/20.0f) * (1 + (x2/42.0f) * (-1+x2/72.0f))));
}

float normal_sin(float x) {
    float res=0, pow=x, fact=1;
    for(int i=0; i<RES; ++i)
    {
        res+=pow/fact;
        pow*=-1*x*x;
        fact*=(2*(i+1))*(2*(i+1)+1);
    }
    return res;
}

float sin(float x)
{
    x = normalize_angle(x);
#if USE_FAST_MATH
    return faster_sin(x);
#else
    return normal_sin(x);
#endif
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

void fft(float data_re[], float data_im[], const unsigned int N) {
    // N must be a power of 2 !!!
    if ((N & (N - 1)) != 0) {
        panic("N must be a power of 2 !!!\n");
        return;
    }

    rearrange(data_re, data_im, N);
    compute(data_re, data_im, N);
}

void rearrange(float data_re[], float data_im[], const unsigned int N) {
    unsigned int target = 0;
    for(unsigned int position = 0; position < N; position++) {
        if(target > position) {
            const float temp_re = data_re[target];
            const float temp_im = data_im[target];
            data_re[target] = data_re[position];
            data_im[target] = data_im[position];
            data_re[position] = temp_re;
            data_im[position] = temp_im;
        }
        unsigned int mask = N;
        while(target & (mask >>= 1)) {
            target &= ~mask;
        }
        target |= mask;
    }
}

void compute(float data_re[], float data_im[], const unsigned int N) {
    const float pi = -PI; // Using PI constant already defined in math.h
    
    for(unsigned int step = 1; step < N; step <<= 1) {
        const unsigned int jump = step << 1;
        const float step_d = (float)step;
        float twiddle_re = 1.0;
        float twiddle_im = 0.0;
        
        for(unsigned int group = 0; group < step; group++) {
            for(unsigned int pair = group; pair < N; pair += jump) {
                const unsigned int match = pair + step;
                const float product_re = twiddle_re * data_re[match] - twiddle_im * data_im[match];
                const float product_im = twiddle_im * data_re[match] + twiddle_re * data_im[match];
                data_re[match] = data_re[pair] - product_re;
                data_im[match] = data_im[pair] - product_im;
                data_re[pair] += product_re;
                data_im[pair] += product_im;
            }
            
            // Skip twiddle factor computation for last iteration of the group
            if(group + 1 == step) {
                continue;
            }

            float angle = pi * ((float)group + 1) / step_d;
            twiddle_re = cos(angle);
            twiddle_im = sin(angle);
        }
    }
}

float sqrt(float x) {
    if (x < 0) return 0;  // Handle invalid input
    if (x == 0) return 0;
    
    float result = x;
    
    // Simple Newton's method
    for(int i = 0; i < 10; i++) {
        result = (result + x/result) * 0.5f;
    }
    
    return result;
}