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
```

```python
BASE_DIR = Path("../")

FILES = [
    "res_equeue3.csv",
    "res_ffqueue2.csv",
]

def load_results(path: str):
    df = pd.read_csv(BASE_DIR.joinpath(path).resolve())

    df["RX_TIME"] = df["RX_End"] - df["RX_Start"]
    df["TX_TIME"] = df["TX_End"] - df["TX_Start"]
    df["LATENCY"] = df["RX_End"] - df["TX_End"]

    return df.groupby("Wait_Time")[["RX_TIME", "TX_TIME", "LATENCY"]].median().reset_index().sort_values("Wait_Time")

results = [(load_results(f), f) for f in FILES]

for res in results:
    print(f"Mean for {res[1]}:\n{res[0][["RX_TIME", "TX_TIME"]].median()}")
    print(f"Std. Deriv. for {res[1]}:\n{res[0][["RX_TIME", "TX_TIME"]].std()}")

```

```python
fig, axs = plt.subplots(1, 3, figsize=(15, 5), sharex=True)

for i, col in enumerate(["RX_TIME", "TX_TIME", "LATENCY"]):
    for res, f in results:
        axs[i].plot(res["Wait_Time"] / 1000, res[col], label=f)
    axs[i].set_xlabel("Wait Time μs")
    axs[i].set_ylabel(f"{col} ns")
    axs[i].legend()

plt.show()
```
