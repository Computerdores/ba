#include <unistd.h>
#include <x86intrin.h>

#include <ctime>
#include <iostream>
#include <thread>

#include "utils.h"

int main() {
    u64 start_time0 = get_clock_timestamp();
    auto start_tsc = __rdtsc();
    u64 start_time1 = get_clock_timestamp();
    u64 start_time = (start_time0 + start_time1) / 2;

    busy_wait(2 * 1000 * 1000 * 1000);

    u64 end_time0 = get_clock_timestamp();
    auto end_tsc = __rdtsc();
    u64 end_time1 = get_clock_timestamp();
    u64 end_time = (end_time0 + end_time1) / 2;

    double time_delta = static_cast<double>(end_time - start_time) / 1000000000.0;
    auto tsc_delta = end_tsc - start_tsc;

    std::cout << "time diff: " << time_delta << std::endl;
    std::cout << " tsc diff: " << tsc_delta << std::endl;
    std::cout << "     freq: " << std::fixed << tsc_delta / time_delta << std::endl;
}
