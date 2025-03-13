#include "fft.h"

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

int fft(float data_re[], float data_im[], const unsigned int N) {
    // N must be a power of 2 !!!
    if ((N & (N - 1)) != 0) {
        panic("N must be a power of 2 !!!\n");
        return -1;
    }

    for (int i = 0; i < 128; i ++){
        data_im[i] = 0;
    }

    // printk("Data in fft: \n");
    // for (int i = 0; i < 128; i++) {
    //     // if (received_data[i] == -1) {
    //         // break;
    //     // }
    //     printk("%f %f", data_re[i], data_im[i]);
    //     printk("\n");
    // }
        

    rearrange(data_re, data_im, N);

    // printk("After rearrange: \n");
    // for (int i = 0; i < 128; i++) {
    //     // if (received_data[i] == -1) {
    //         // break;
    //     // }
    //     printk("%f %f", data_re[i], data_im[i]);
    //     printk("\n");
    // }
        

    // for (int i = 0; i < (N + 7-1)/7; i ++){
    //     for (int j = 0; j < 7; j ++){
    //         printk("%f ", data_re[i*7 + j]);
    //     }
    //     printk("\n");
    //     // printk("Data index %d is %f\n",i, data_re[i]);
    // }

    // float* real = kmalloc(N * sizeof(float));
    float real[N];
    float imag[N];
    for (int i = 0; i < 128; i ++){
        real[i] = data_re[i];
        imag[i] = data_im[i];
    }
    // memcpy(real, data_re, N * sizeof(float));
    // memset()
    // float* imag = kmalloc(N * sizeof(float));
    // float imag[N];
    // memcpy(imag, data_im, N * sizeof(float));





    int max_index = compute(real, imag, N);


    // for (int i = 0; i < (N + 7-1)/7; i ++){
    //     for (int j = 0; j < 7; j ++){
    //         printk("%f ", real[i*7 + j]);
    //     }
    //     printk("\n");
    //     // printk("Data index %d is %f\n",i, data_re[i]);
    // }

    // our signal generator can't output below 40 hz
    // if (max_index < 6){
    //     for (int i = 0; i < N; i ++){
    //         // for (int j = 0; j < 7; j ++){
    //         //     printk("%f ", real[i*7 + j] * real[i*7 + j] + imag[i*7 + j] * imag[i*7 + j]);
    //         // }
    //         // printk("\n");

    //         printk("magn index %d = %f \n",i,  real[i] * real[i] + imag[i] * imag[i]);
    //         // printk("Data index %d is %f\n",i, data_re[i]);
    //     }
    // }

    // if (max_index < 6){
    //     for (int i = 0; i < N; i ++){
    //         // for (int j = 0; j < 7; j ++){
    //         //     printk("%f ", real[i*7 + j] * real[i*7 + j] + imag[i*7 + j] * imag[i*7 + j]);
    //         // }
    //         // printk("\n");

    //         printk("real index %d = %f \n",i,  real[i]);
    //         // printk("Data index %d is %f\n",i, data_re[i]);
    //     }
    // }


    return max_index;
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

int compute(float data_re[], float data_im[], const unsigned int N) {
    const float pi = -PI;
    int max_index = 0;
    float max_magnitude = 0;
    
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

                // Check magnitude after final butterfly stage
                if (step == N/2) {
                    // Only check first half of frequencies
                    if (pair < N/2) {
                        float magnitude = data_re[pair] * data_re[pair] + data_im[pair] * data_im[pair];
                        
                        if ((magnitude > max_magnitude) && (pair > 0)) {
                            max_magnitude = magnitude;
                            max_index = pair;
                            printk("max index is %d \n", max_index);
                        }
                        // printk("magnitude index %d is %f \n", pair, magnitude);
                        // printk("Magnitude 2 is %f, %f \n", pair*860/(2*128),magnitude);
                    }
                    if (match < N/2) {
                        float magnitude = data_re[match] * data_re[match] + data_im[match] * data_im[match];
                        if ((magnitude > max_magnitude) && (match > 0)) {
                            max_magnitude = magnitude;
                            max_index = match;
                        }
                    }
                }
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
    
    return max_index;
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