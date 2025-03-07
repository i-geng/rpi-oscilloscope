
#include "rpi.h"
#include "math.h"

void notmain(void) {
    float x[96];
    for (int i = 0; i < 96; i++) {
        float theta = (2 * i * PI / 32.0f);
        // Normalize theta to [-pi, pi]
        theta = fmod(theta + PI, 2 * PI) - PI;
        x[i] = sin(theta);
    }
    // Print the signal
    printk("Signal generated\n");

    printk("Signal visualization:\n");
    plot_signal(x, 96, 1.0f, 20.0f);
    printk("Signal visualization complete\n");
}