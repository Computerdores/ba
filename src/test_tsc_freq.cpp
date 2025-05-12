#include <cassert>
#include <x86intrin.h>
#include <unistd.h>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <thread>

int main() {
    timespec start_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
    auto start_tsc = __rdtsc();

    sleep(2);

    timespec end_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &end_time);
    auto end_tsc = __rdtsc();

    double time_delta = (end_time.tv_sec * 1000000000.0 + end_time.tv_nsec - (start_time.tv_nsec + start_time.tv_sec * 1000000000.0)) / 1000000000.0;
    auto tsc_delta = end_tsc - start_tsc;

    std::cout << "time diff: " << time_delta << std::endl;
    std::cout << " tsc diff: " << tsc_delta << std::endl;
    std::cout << "     freq: " << std::fixed << tsc_delta / time_delta << std::endl;
}