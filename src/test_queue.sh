#!/usr/bin/env bash

ITERATIONS=50

set -euo pipefail

cmake --build build/

sudo cpupower frequency-set -f "2.8G" >/dev/null

datafile=$(mktemp)
trap 'rm -f "$datafile"' EXIT
analysisfile=$(mktemp)
trap 'rm -f "$analysisfile"' EXIT

total_rx="0"
total_tx="0"
for i in $(seq 1 $ITERATIONS); do
    build/benchmarks -o "$datafile" $@
    jupyter/print_basic_stats.py "$datafile" | tee "$analysisfile"

    rx_mean=$(cat "$analysisfile" | awk '/Mean:/{flag=1;next} flag && /RX_TIME/{print $2; exit}')
    tx_mean=$(cat "$analysisfile" | awk '/Mean:/{flag=1;next} flag && /TX_TIME/{print $2; exit}')

    echo "Iteration $i: RX mean = $rx_mean, TX mean = $tx_mean"
    
    total_rx=$(LC_ALL=C awk "BEGIN {print $total_rx+$rx_mean}")
    total_tx=$(LC_ALL=C awk "BEGIN {print $total_tx+$tx_mean}")
done

echo "RX Mean: $(LC_ALL=C awk "BEGIN {print $total_rx/$ITERATIONS}")"
echo "TX Mean: $(LC_ALL=C awk "BEGIN {print $total_tx/$ITERATIONS}")"

sudo cpupower frequency-set -g powersave >/dev/null
