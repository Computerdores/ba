#!/usr/bin/env python3
import sys

import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import pandas as pd

from matplotlib.axes import Axes
from pathlib import Path

from lib import load_results

QUEUES = ["bq", "eq", "mcrb", "fflwq", "ffwdq", "lprt"]
RESULTS_DIR = Path("data/")

ERROR_BARS = False

def get_result(benchmark: str, queue: str, preset: str, jitter: bool, all: bool, runs: int = 100) -> tuple[float, float]:
    results = []
    for i in range(runs):
        filename = f"data_{preset}_{benchmark}_{queue}_j{str(jitter).lower()}_mf{str(all).lower()}_{i+1}.csv"
        df = load_results(RESULTS_DIR / filename)
        mean = df[["RX_TIME", "TX_TIME"]].mean()
        results.append({
            "queue": queue,
            "run": i,
            "RX": mean["RX_TIME"],
            "TX": mean["TX_TIME"],
        })
    return pd.DataFrame(results)

def gen_plot(benchmark: str, queues: list[str], preset: str, jitter: bool, all: bool):
    results = [get_result(benchmark, q, preset, jitter, all) for q in queues]
    df = pd.concat(results, ignore_index=True)

    df = df.melt(
        id_vars=["queue", "run"],
        value_vars=["RX", "TX"],
        var_name="type",
        value_name="duration"
    )

    plt.figure(figsize=(8,6))
    ax: Axes = sns.barplot(data=df, x="queue", y="duration", hue="type", errorbar=("ci", 95), capsize=0.1)

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
            print(f"{benchmark} {jitter} {all} - done")

if __name__ == "__main__":
    if not len(sys.argv) > 1:
        print(f"Usage: {sys.argv[0]} <preset>")
    else:
        for benchmark in ["basic", "bursty"]:
            gen_plot_quad(benchmark, QUEUES, sys.argv[1])
