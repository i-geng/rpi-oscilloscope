import numpy as np
import matplotlib.pyplot as plt

samples = 5 # must be greater than frequency * 2 NOT GREATER OR EQUAL.
upsamples = 100
ts = 1/samples
freq = 1 # signal goes up to frequency * 2
periods = 2

# Take dft
def dft(x):
    N = len(x)
    X = []

    # Iterate over all frequency bins
    for k in range(N):
        X_k = 0

        # Accumulate on frequency bin
        for n in range(N):
            e = np.exp(2j * np.pi * k * n /N)
            X_k += x[n]/e
        
        X.append(X_k)

    return np.array(X)

# Take inverse dft
def idft(X):
    N = len(X)
    x = []

    # Iterate over all time samples
    for n in range(N):
        x_n = 0

        # Accumulate on time sample
        for k in range(N):
            e = np.exp(2j * np.pi * k * n /N)/N
            x_n += X[k] * e
        
        x.append(x_n)

    return np.array(x)


# Arbitrary signal
s1= np.sin(2* np.pi * freq * np.arange(0,1 * periods,ts))
s2 = 0.5*np.sin(2* np.pi * 2 * freq * np.arange(0,1 * periods,ts))
s3 = 1.5*np.sin(2* np.pi * 0.5 * freq * np.arange(0,1 * periods,ts))

signal = s1 + s2 + s3

# Take transform
transform = dft(signal)

# Pad: Need to maintain current bins. 40 time samples lead to 40 frequency samples, to upscale we decrease the latter.
# We can increase frequency samples arbitrarily by padding inbetween the DC/Nyquist bins. What this means:
# Padded[0:21] = old[0:21]
# Padded[N-19:N] = old[22:40]
# Everything inbetween set to 0
padded = np.zeros(upsamples * periods, dtype = "complex")
left_size = (samples * periods)//2 + 1  # e.g., for N=40, left_size=21
right_size = (samples * periods) - left_size  # e.g., for N=40, right_size=19
padded[0:left_size] = transform[0:left_size]
if right_size > 0:
    padded[upsamples * periods - right_size:] = transform[-right_size:]


fig, axs = plt.subplots(2)
fig.suptitle('Original, sampled, and reconstructed signals')

# Plot original
axs[0].plot(np.linspace(0, samples * periods, upsamples * periods), 
            np.sin(2* np.pi * freq * np.arange(0,1 * periods,1/upsamples)) +
            0.5*np.sin(2* np.pi * 2 * freq * np.arange(0,1 * periods,1/upsamples)) +
            1.5*np.sin(2* np.pi * 0.5 * freq * np.arange(0,1 * periods,1/upsamples)))


# Plot sampled and reconstructed
axs[1].scatter(range(samples * periods), signal, c = 'r')
axs[1].plot(np.linspace(0, samples * periods, upsamples * periods), idft(padded) * upsamples / samples, c = 'b')
plt.savefig("dft.jpg")