---
jupyter:
  jupytext:
    formats: ipynb,md
    text_representation:
      extension: .md
      format_name: markdown
      format_version: '1.3'
      jupytext_version: 1.16.4
  kernelspec:
    display_name: Python 3
    language: python
    name: python3
---

```python
%matplotlib widget
```

```python
import matplotlib.pyplot as plt
from pathlib import Path
import seaborn as sns

from lib import load_results
```

```python
plt.close("all")

BASE_DIR = Path("./")

FILES = [
    "flugzeug_basic_bq_9f53673_clean.csv",
    "flugzeug_basic_eq_9f53673_clean.csv",
    "flugzeug_basic_ffq_9f53673_clean.csv",
    "flugzeug_basic_ffwdq_9f53673_clean.csv",
    "flugzeug_basic_lprt_9f53673_clean.csv",
    "flugzeug_bursty_bq_9f53673_clean.csv",
    "flugzeug_bursty_eq_9f53673_clean.csv",
    "flugzeug_bursty_ffq_9f53673_clean.csv",
    "flugzeug_bursty_ffwdq_9f53673_clean.csv",
    "flugzeug_bursty_lprt_9f53673_clean.csv",
]

MOD = 10

BY_QUEUE = True    # whether OFF selects by QUEUE or by RUN
OFF = 3
results = [(load_results(BASE_DIR.joinpath(f)), f) for i, f in enumerate(FILES) if (i % MOD if BY_QUEUE else i // MOD) == OFF]
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
