
#include <iostream>

#include "utils.h"

/**
 * Tests the difference between @ref get_tsc_timestamp and @ref get_clock_timestamp.
 * @return Difference in percent (e.g. 10% = 0.1).
 */
double test_timestamp_difference() {
    const u64 tsc_ts0 = get_tsc_timestamp();
    const u64 clock_ts0 = get_clock_timestamp();

    busy_wait(2 * 1'000'000'000);
    asm volatile ("" ::: "memory");

    const u64 tsc_ts1 = get_tsc_timestamp();
    const u64 clock_ts1 = get_clock_timestamp();

    const u64 tsc_diff = tsc_ts1 - tsc_ts0;
    const u64 clock_diff = clock_ts1 - clock_ts0;
    return static_cast<double>(std::max(tsc_diff, clock_diff)) / static_cast<double>(std::min(tsc_diff, clock_diff)) -
           1.0;
}

// Runs the timestamp difference test, prints the result and returns success if the difference is below 0.01%
int main() {
    const auto diff = test_timestamp_difference();
    std::cout << "Difference: " << diff * 100.0 << "%" << std::endl;
    return diff > 0.0001;
}
