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
    display_name: .venv (3.12.3)
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
plt.close("all")

BASE_DIR = Path("../")

FILES = [
    "flugzeug_basic_bq_5f9af44_jitter.csv",
    "flugzeug_basic_eq_5f9af44_jitter.csv",
    "flugzeug_basic_ffq_5f9af44_jitter.csv",
    "flugzeug_bursty_bq_5f9af44_jitter.csv",
    "flugzeug_bursty_eq_5f9af44_jitter.csv",
    "flugzeug_bursty_ffq_5f9af44_jitter.csv",
]

def load_results(path: str):
    df = pd.read_csv(BASE_DIR.joinpath(path))

    df["RX_TIME"] = df["RX_End"] - df["RX_Start"]
    df["TX_TIME"] = df["TX_End"] - df["TX_Start"]
    df["LATENCY"] = df["RX_End"] - df["TX_End"]

    return df

MOD = 6

BY_QUEUE = False    # whether OFF selects by QUEUE or by RUN
OFF = 0
results = [(load_results(f), f) for i, f in enumerate(FILES) if (i % MOD if BY_QUEUE else i // MOD) == OFF]

for res in results:
    print(res[1])
    avg_rx_diff, avg_tx_diff = res[0]["RX_Start"].diff().dropna().mean(), res[0]["TX_Start"].diff().dropna().mean()
    print(f"TX Wait: {avg_tx_diff:.2f}, RX Wait: {avg_rx_diff:.2f}")
    rx_rate, tx_rate = 1000000000 / avg_rx_diff, 1000000000 / avg_tx_diff
    print(f"TX Rate: {tx_rate:.2f}, RX_Rate: {rx_rate:.2f}"),
    print(f"Rate Diff: {tx_rate - rx_rate:.2f}")
    print(f"Mean:\n{res[0][["RX_TIME", "TX_TIME"]].mean()}")
    print(f"Median:\n{res[0][["RX_TIME", "TX_TIME"]].median()}")
    print(f"Standard Deviation:\n{res[0][["RX_TIME", "TX_TIME"]].std()}")
    print("---------------------------------------------")
```

```python
#raise RuntimeError
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
#plt.xlim(400000, 600000)
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
#plt.xlim(400000, 600000)
plt.legend()
plt.tight_layout()
plt.show()
```
