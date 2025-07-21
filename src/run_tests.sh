#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <run_name>"
    exit 1
fi

cmake --build build/

sudo cpupower frequency-set -f "2.8G" >/dev/null

for _test in basic bursty; do
    echo "Now running $_test tests"
    for queue in bq eq ffq; do
        "build/$_test" "$queue" > "flugzeug_${_test}_${queue}_$1.csv"
    done
done

sudo cpupower frequency-set -g powersave >/dev/null
