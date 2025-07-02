---
jupyter:
  jupytext:
    formats: ipynb,md
    text_representation:
      extension: .md
      format_name: markdown
      format_version: '1.3'
      jupytext_version: 1.17.2
  kernelspec:
    display_name: .venv
    language: python
    name: python3
---

```python
%matplotlib widget
```

```python
import pandas as pd
import matplotlib.pyplot as plt
```

```python
from pathlib import Path
BASE_DIR = Path("../")

FILES = [
    #"flugzeug_bq_s16384_bs8192_bi64_wt50.csv",
    #"flugzeug_bq_s16384_bs2048_bi64_wt50.csv",
    #"flugzeug_bq_s16384_bs1024_bi64_wt50.csv",
    #"flugzeug_bq_s16384_bs32_bi64_wt50.csv",
    #"flugzeug_bq_s16384_bs8_bi64_wt50.csv",
    #"flugzeug_bq_s16384_bs8192_bi64_wt200.csv",
    #"flugzeug_bq_s16384_bs8192_bi64_wt500.csv",
    "data_eq_8192_16384_50.csv",
    "data_bq_16384_8192_64_50.csv",
    "data_ffq_1024_16.csv",
]

def load_results(path: str):
    df = pd.read_csv(BASE_DIR.joinpath(path))

    df["RX_TIME"] = df["RX_End"] - df["RX_Start"]
    df["TX_TIME"] = df["TX_End"] - df["TX_Start"]
    df["LATENCY"] = df["RX_End"] - df["TX_End"]

    return df

results = [(load_results(f), f) for f in FILES]

for res in results:
    print(f"Mean for {res[1]}:\n{res[0][["RX_TIME", "TX_TIME"]].mean()}")
    print(f"Median for {res[1]}:\n{res[0][["RX_TIME", "TX_TIME"]].median()}")
    print(f"Std. Deriv. for {res[1]}:\n{res[0][["RX_TIME", "TX_TIME"]].std()}")
    print("---------------------------------------------")

```

```python
import seaborn as sns

# Plot RX_TIME
plt.figure(figsize=(10, 6))
for df, filename in results:
    filtered_rx = df["RX_TIME"][df["RX_TIME"] <= 200]
    sns.histplot(filtered_rx, bins=50, label=filename, element="step")
plt.title("RX_TIME Histogram (≤100)")
plt.xlabel("RX_TIME")
plt.ylabel("Density")
plt.xlim(0, 200)
plt.legend()
plt.tight_layout()
plt.show()

# Plot TX_TIME
plt.figure(figsize=(10, 6))
for df, filename in results:
    filtered_tx = df["TX_TIME"][df["TX_TIME"] <= 200]
    sns.histplot(filtered_tx, bins=50, label=filename, element="step")
plt.title("TX_TIME Histogram (≤100)")
plt.xlabel("TX_TIME")
plt.ylabel("Density")
plt.xlim(0, 200)
plt.legend()
plt.tight_layout()
plt.show()

```
