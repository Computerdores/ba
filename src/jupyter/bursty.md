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
from pathlib import Path
import seaborn as sns
```

```python
BASE_DIR = Path("../")

FILES = [
    "flugzeug_nowarmup_bq_16384_8192_64_50.csv",
    #"flugzeug_nowarmup_eq_8192_16384_50.csv",
    #"flugzeug_nowarmup_ffq_1024_16.csv",
    "flugzeug_nowarmup_bq_16384_8192_64_50_new.csv",
    #"flugzeug_nowarmup_eq_8192_16384_50_new.csv",
    #"flugzeug_nowarmup_ffq_1024_16_new.csv",
    #"flugzeug_warmup_bq_16364_8192_64_50.csv",
    #"flugzeug_warmup_eq_4096_16384_50.csv",
    #"flugzeug_warmup_ffq_1024_16.csv",
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
# Plot RX_TIME
plt.figure(figsize=(10, 6))
for df, filename in results:
    filtered_rx = df["RX_TIME"][df["RX_TIME"] <= 200]
    sns.histplot(filtered_rx, bins=50, label=filename, element="step")
plt.title("RX_TIME Histogram")
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
plt.title("TX_TIME Histogram")
plt.xlabel("TX_TIME")
plt.ylabel("Density")
plt.xlim(0, 200)
plt.legend()
plt.tight_layout()
plt.show()
```

```python
# Plot RX_TIME
plt.figure(figsize=(10, 6))
for df, filename in results:
    filtered_rx = df["RX_TIME"][df["RX_TIME"] <= 200]
    sns.lineplot(x=filtered_rx.index, y=filtered_rx, label=filename)
plt.title("RX_TIME over Time")
plt.xlabel("Message Index")
plt.ylabel("RX_TIME")
plt.legend()
plt.tight_layout()
plt.show()

# Plot TX_TIME
plt.figure(figsize=(10, 6))
for df, filename in results:
    filtered_tx = df["TX_TIME"][df["TX_TIME"] <= 200]
    sns.lineplot(x=filtered_tx.index, y=filtered_tx, label=filename)
plt.title("TX_TIME over Time")
plt.xlabel("Message Index")
plt.ylabel("TX_TIME")
plt.legend()
plt.tight_layout()
plt.show()
```
