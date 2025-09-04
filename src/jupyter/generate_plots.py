#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import pandas as pd

from matplotlib.axes import Axes
from pathlib import Path

from lib import load_results

RESULTS_DIR = Path(".")

ERROR_BARS = False

def get_result(benchmark: str, queue: str, preset: str, jitter: bool, all: bool) -> tuple[float, float]:
    filename = f"data_{preset}_{benchmark}_{queue}_j{str(jitter).lower()}_mf{str(all).lower()}.csv"
    df = load_results(RESULTS_DIR / filename)
    # TODO: determine mean and stddev over multiple runs
    mean = df[["RX_TIME", "TX_TIME"]].mean()
    std = df[["RX_TIME", "TX_TIME"]].std() / 100 # TODO: this division is only here so the graphs can be worked on in the mean time
    return (mean["RX_TIME"], std["RX_TIME"]), (mean["TX_TIME"], std["TX_TIME"])

def gen_plot(benchmark: str, queues: list[str], preset: str, jitter: bool, all: bool):
    results = [(q, get_result(benchmark, q, preset, jitter, all)) for q in queues]

    data = [
        {"queue": q, "type": t, "mean": r[i][0], "std": r[i][1]}
        for q, r in results
        for i, t in enumerate(["RX", "TX"])
    ]

    df = pd.DataFrame(data)

    plt.figure(figsize=(8,6))
    ax: Axes = sns.barplot(data=df, x="queue", y="mean", hue="type", errorbar=None, capsize=0.1)

    if ERROR_BARS:
        for i, row in df.iterrows():
            # Get the x-position of the bar
            x = i//2 + (-0.2 if row["type"]=="RX" else 0.2)
            ax.errorbar(x, row["mean"], yerr=row["std"], fmt='none', c='black', capsize=5)

    plt.ylabel("Duration (ns)")
    title = f"RX and TX for each Queue {"with" if jitter else "without"} jitter and while{"" if all else " not"} measuring failed operations"
    plt.title(title)
    plt.legend(title="Type")
    plt.tight_layout()
    plt.savefig(f"{preset}_{benchmark}_{jitter}_{all}.png")

def gen_plot_quad(benchmark: str, queues: list[str], preset: str):
    for jitter in [True, False]:
        for all in [True, False]:
            gen_plot(benchmark, queues, preset, jitter, all)

if __name__ == "__main__":
    gen_plot_quad("bursty", ["bq", "eq", "fflwq", "ffwdq", "lprt"], "5157004_16k")
