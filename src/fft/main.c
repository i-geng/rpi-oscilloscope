#include "rpi.h"
#include "math.h"

// Test with a power of 2
#define N 1024

void print_complex_array(const char* name, float* re, float* im, unsigned int n) {
    printk("%s:\n", name);
    for(unsigned int i = 0; i < n; i++) {
        if (im[i] >= 0) {
            printk("[%d]: %d + %di\n", i, (int)(re[i] * 1000), (int)(im[i] * 1000));
        } else {
            printk("[%d]: %d - %di\n", i, (int)(re[i] * 1000), (int)(-im[i] * 1000));
        }
    }
    printk("\n");
}

void generate_test_signal(float* data_re, float* data_im) {
    // Generate a signal with two frequency components:
    // 1. A slower wave with frequency 1/16 of sampling rate
    // 2. A faster wave with frequency 1/4 of sampling rate
    
    for(unsigned int i = 0; i < N; i++) {
        // Slow component: cos(2π * i/16)
        // float slow = cos(TWO_PI * i / 16.0);
        float slow = 0.0 * cos(TWO_PI * i / 13.0);
        
        // Fast component: 0.5 * cos(2π * i/4)
        // float fast = 1.5 * cos(TWO_PI * i / 4.0);
        float fast = 1.5 * cos(TWO_PI * i / 3.0);
        
        // Combine components
        data_re[i] = slow + fast;
        data_im[i] = 0;  // Start with no imaginary component
    }
}

void notmain() {
    printk("FFT Test Program (N=%d points)\n\n", N);

    // Allocate arrays
    float data_re[N];
    float data_im[N];

    // Time signal generation
    uint32_t start_time = timer_get_usec();
    
    generate_test_signal(data_re, data_im);


    // Plot input signal
    // printk("Input signal plot:\n");
    // plot_signal(data_re, N, 0, 20);
    // printk("\n");

    // Time FFT computation
    start_time = timer_get_usec();
    
    int max_index = fft(data_re, data_im, N);
    

    uint32_t fft_time = timer_get_usec() - start_time;
    printk("FFT computation time: %d microseconds\n\n", fft_time);

    // Print magnitude of FFT result for first N/2 frequencies
    // (since FFT output is symmetric for real input)
    printk("FFT magnitude spectrum:\n");
    
    // Time magnitude computation
    start_time = timer_get_usec();
    
    float magnitude[N/2];
    for(unsigned int i = 0; i < N/2; i++) {
        magnitude[i] = data_re[i] * data_re[i] + data_im[i] * data_im[i];
        if (magnitude[i] > 10) {
            printk("Frequency bin %d: %d\n", i, (int)(magnitude[i] * 1000));
        }
    }
    printk("Max index: %d\n", max_index);
    // printk("Magnitude spectrum plot:\n");
    // plot_signal(magnitude, N/2, 0, 2);
    printk("FFT test completed!\n");
}