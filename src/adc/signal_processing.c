#include <complex.h>
#include <math.h>
#include "rpi.h"
#include "adc.h"

#define pi           3.14159265358979323846

#define samples     5
#define upsamples   100
#define ts          1.0/samples
#define freq        1
#define periods     2

float* dft(float* x, uint16_t N){

    // malloc spectrum
    float* X = kmalloc(sizeof(float) * N);

    // Iterate over all frequency bins
    for (int k = 0; k < N; k ++){
        X[k] = 0;
    
        // Iterate over time 
        for (int n = 0; n < N; n ++){
            float e = exp(2 * I * pi * k * n / (1.0 * N));
            X[k] += x[n]/e;
        }
    }

    return X;
}

float* upsample_spectrum(float* X, uint16_t N, uint16_t M){

    // allocate array for padding
    float* padded = kmalloc(sizeof(float) * M);

    int left_size = (N)/2 + 1;
    int right_size = M - left_size;

    for (int i = 0; i < left_size; i ++){
        padded[i] = X[i];
    }

    for (int i = (M - right_size); i < M; i ++){
        padded[i] = X[i - (M - right_size) + left_size];
    }

    return padded;
}

// M SHOULD BE HIGHER THAN N TO INTERPOLATE
float* idft(float* X, uint16_t M){
    
    // malloc time
    float* x = kmalloc(sizeof(float) * M);

    // iterate over all time bins
    for (int n = 0; n < M; n ++){
        x[n] = 0;

        // Iterate over frequency
        for (int k = 0; k < M; k ++){
            float e = exp(2 * I * pi * k * n / (1.0 * M))/(1.0 * M);
            x[n] += X[k] * e;
        }
    }
    return x;
}

void notmain(){

    kmalloc_init();
    
    // Define an arbitrary signal
    float* s1 = kmalloc(sizeof(float) * periods/ts);
    float* s2 = kmalloc(sizeof(float) * periods/ts);
    float* s3 = kmalloc(sizeof(float) * periods/ts);
    float* signal = kmalloc(sizeof(float) * periods/ts);

    for (int i = 0; i < periods / ts; i ++){
        s1[i] = 1.0 * sin(2 * pi * 1.0 * freq * i * ts);
        s2[i] = 0.5 * sin(2 * pi * 2.0 * freq * i * ts);
        s3[i] = 2.0 * sin(2 * pi * 0.5 * freq * i * ts);

        // Add everything up
        signal[i] = s1[i] + s2[i] + s3[i];
    }

    // Take dft
    float* X = dft(signal, samples * periods);

    // upsample
    float* upsampled_X = upsample_spectrum(X, samples * periods, upsamples * periods);

    // Take idft
    float* out = idft(X, upsamples * periods);

}