#!/usr/bin/env python3
import sys

from lib import load_results

print("---------------------------------------------")
for f in sys.argv[1:]:
    res = load_results(f)
    print(f)
    avg_rx_diff, avg_tx_diff = res["RX_Start"].diff().dropna().mean(), res["TX_Start"].diff().dropna().mean()
    print(f"TX Wait: {avg_tx_diff:.2f}, RX Wait: {avg_rx_diff:.2f}")
    rx_rate, tx_rate = 1000000000 / avg_rx_diff, 1000000000 / avg_tx_diff
    print(f"TX Rate: {tx_rate:.2f}, RX_Rate: {rx_rate:.2f}"),
    print(f"Rate Diff: {tx_rate - rx_rate:.2f}")
    print(f"Mean:\n{res[["RX_TIME", "TX_TIME"]].mean()}")
    print(f"Median:\n{res[["RX_TIME", "TX_TIME"]].median()}")
    print(f"Standard Deviation:\n{res[["RX_TIME", "TX_TIME"]].std()}")
    print("---------------------------------------------")
