#!/usr/bin/env bash

if [ -z "$1" ]; then
    echo "Usage: $0 <run_name>"
    exit 1
fi

CPUPOWER="cpupower"

ITERATIONS=100
CONFIG=""
DRY=true

run_name="$(git rev-parse --short HEAD)_$1"
echo "run name: $run_name"

set -e

cmake --build build/ -- -j 8

build/test_timestamp_methods || { echo "Aborting: The get_tsc_timestamp method has a larger difference to the get_clock_timestamp method than expected." >&2; exit 1; }

sudo "$CPUPOWER" frequency-set -f "2.8G" >/dev/null

for benchmark in basic bursty; do
    for queue in bq eq mcrb fflwq ffwdq lprt; do
        echo "Now running $benchmark $queue"
        for jitter in true false; do
            for measure_failed in true false; do
                for i in $(seq 1 $ITERATIONS); do
                    if $DRY; then
                        echo -e "Would have run with: $run_name $benchmark $queue j:$jitter mf:$measure_failed i:$i"
                    else
                        build/benchmarks -c "$CONFIG" -q "$queue" -b "$benchmark" -o "data/data_${run_name}_${benchmark}_${queue}_j${jitter}_mf${measure_failed}_${i}.csv" --jitter=$jitter --measure-failed=$measure_failed
                    fi
                done
            done
        done
    done
done

sudo "$CPUPOWER" frequency-set -g powersave >/dev/null
