import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv('adc_data.csv')
print(df)

print(df.head())

# plot the data
plt.plot(df['time'], df['data'])
plt.show()
plt.savefig('adc_data.png')