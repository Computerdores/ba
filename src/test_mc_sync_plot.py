import pandas as pd
import matplotlib.pyplot as plt

COL1 = "tsc diff";
COL2 = "CLOCK_MONOTONIC_RAW diff";

# Load the CSV
df = pd.read_csv("res.csv")

col1 = df[COL1][df[COL1] < 0.0005e6]
col2 = df[COL2][df[COL2] < 0.0005e6]

# Create subplots
fig, axs = plt.subplots(1, 2, figsize=(12, 5))

# Plot frequency histograms
axs[0].hist(col1, bins=50, color='blue', edgecolor='black')
axs[0].set_title(COL1)
axs[0].set_xlabel('Value')
axs[0].set_ylabel('Frequency')

axs[1].hist(col2, bins=50, color='green', edgecolor='black')
axs[1].set_title(COL2)
axs[1].set_xlabel('Value')
axs[1].set_ylabel('Frequency')

plt.tight_layout()
plt.show()

