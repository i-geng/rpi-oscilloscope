#include "signal_processing.h"


// Generate something that's approximately random
float* generate_dummy_signal(uint16_t samples,  uint16_t freq){

    // timestep
    float ts = 1.0/samples; 

    // Define an arbitrary signal
    float* s1 = kmalloc(sizeof(float) * samples);
    float* s3 = kmalloc(sizeof(float) * samples);
    float* s2 = kmalloc(sizeof(float) * samples);
    float* signal = kmalloc(sizeof(float) * samples);

    for (int i = 0; i < samples; i ++){
        s1[i] = 1.0 * csin(2 * pi * 1.0 * freq * i * ts);
        s2[i] = 0;//0.5 * csin(2 * pi * 2.0 * freq * i * ts);
        s3[i] = 0;//1.5 * csin(2 * pi * 0.5 * freq * i * ts);

        // Add everything up
        signal[i] = s1[i] + s2[i] + s3[i];
    }

    return signal;
}

// Take dft of a real signal, returning a complex spectrum
float complex* dft(float* x, uint16_t N){

    // malloc spectrum
    float complex* X = kmalloc(sizeof(*X) * N);

    // Iterate over all frequency bins
    for (int k = 0; k < N; k ++){
        X[k] = 0;
    
        // Iterate over time 
        for (int n = 0; n < N; n ++){
            float complex e = cexp(-2 * I * pi * k * n / (1.0 * N));
            X[k] += x[n]*e;
        }
    }

    return X;
}

// Upsample the spectrum to interpolate in time
float complex* upsample_spectrum(complex float* X, uint16_t N, uint16_t M){

    // allocate array for padding
    float complex* padded = kmalloc(sizeof(*padded) * M);

    int left_size = (N)/2 + 1;
    int right_size = N - left_size;

    // printk("left size is %d, right size is %d \n", left_size, right_size);

    for (int i = 0; i < M; i ++){
        if (i < left_size)
            padded[i] = X[i];
        else if (i >= (M - right_size))
            padded[i] = X[left_size + i - (M - right_size)];
        else
            padded[i] = 0;
    }

    return padded;
}

// Take the idft of a complex spectrum, returning a real signal
float* idft(complex float* X, uint16_t M, uint16_t N){
    
    // malloc time
    float* x = kmalloc(sizeof(*x) * M);

    // iterate over all time bins
    for (int n = 0; n < M; n ++){
        x[n] = 0;

        // Iterate over frequency
        for (int k = 0; k < M; k ++){
            float complex e = cexp(2 * I * pi * k * n / (1.0 * M))/(1.0 * M);
            x[n] += X[k] * e * M / N;
        }
    }
    return x;
}

// we're so silly we're so silly we're so silly we're so silly we're so silly
// we're so silly we're so silly we're so silly we're so silly we're so silly
// we're so silly we're so silly we're so silly we're so silly we're so silly
float* ENHANCE(float* signal, uint16_t N, uint16_t M){

    float* output;

    // Take dft
    float complex* X = dft(signal, N);

    // Upsample
    float complex* X_upsampled = upsample_spectrum(X, N, M);

    // Take idft
    output = idft(X_upsampled, M, N);

    return output;
}


// void notmain(){

//     kmalloc_init();
    
//     uint16_t freq = 1;
//     uint16_t samples = 5;
//     uint16_t upsamples = 100;

//     // generate dummy
//     float* signal = generate_dummy_signal(samples, freq);

//     // Do the thing
//     float* out = ENHANCE(signal, samples, upsamples);


//     for (int i = 0; i < upsamples; i ++){
//         printk("%f \n", out[i]);
//     }

// }