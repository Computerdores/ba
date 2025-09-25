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

PLOT_ERRORBAR_TYPE = ("ci", 95)

REPORT_HEADER_TEMPLATE = """
# {preset}-{benchmark}-{jitter}-{all}
preset: {preset}
type: {benchmark}
jitter: {jitter}
measure failed: {all}
plot errorbar type: {errorbar_type}

## Results per Queue
| Queue | Type | Mean (ns) | Std Dev (ns) |
|-------|------|-----------|--------------|
"""

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

def gen_plot(df, filename: str, jitter: bool, all: bool):
    plt.figure(figsize=(8,6))
    ax: Axes = sns.barplot(data=df, x="queue", y="duration", hue="type", errorbar=PLOT_ERRORBAR_TYPE, capsize=0.1)

    plt.ylabel("Duration (ns)")
    title = f"RX and TX for each Queue {"with" if jitter else "without"} jitter and while{"" if all else " not"} measuring failed operations"
    plt.title(title)
    plt.legend(title="Type")
    plt.tight_layout()
    plt.savefig(filename)

def gen_report(df, filename: str, preset: str, benchmark: str, jitter: bool, all: bool):
    report = REPORT_HEADER_TEMPLATE.format(
        preset=preset,
        benchmark=benchmark,
        jitter=jitter,
        all=all,
        errorbar_type=PLOT_ERRORBAR_TYPE,
    )

    summary = (
        df.groupby(["queue", "type"])["duration"]
        .agg(["mean", "std"])
        .reset_index()
    )

    for _, row in summary.iterrows():
        report += f"| {row['queue']} | {row['type']} | {row['mean']:.2f} | {row['std']:.2f} |\n"

    report += "\n---\n\n"

    with open(filename, "a") as f:
        f.write(report)

def gen_plot_quad(benchmark: str, queues: list[str], preset: str):
    for jitter in [True, False]:
        for all in [True, False]:
            results = [get_result(benchmark, q, preset, jitter, all) for q in queues]
            df = pd.concat(results, ignore_index=True)

            df = df.melt(
                id_vars=["queue", "run"],
                value_vars=["RX", "TX"],
                var_name="type",
                value_name="duration"
            )

            gen_plot(df, f"{preset}_{benchmark}_{jitter}_{all}.png", jitter, all)
            gen_report(df, f"report_{preset}.md", preset, benchmark, jitter, all)
            print(f"{benchmark} {jitter} {all} - done")

if __name__ == "__main__":
    if not len(sys.argv) > 1:
        print(f"Usage: {sys.argv[0]} <preset>")
    else:
        for benchmark in ["basic", "bursty"]:
            gen_plot_quad(benchmark, QUEUES, sys.argv[1])
