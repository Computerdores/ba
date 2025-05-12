#include <x86intrin.h>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <optional>
#include <thread>

#include "ff_queue.hpp"
#include "utils.h"

#define CPU_MAIN 5
#define CPU_W0   0
#define CPU_W1   2

typedef struct alignas(CACHE_LINE_SIZE) {
    unsigned long long tsc;
    timespec tp;
} Timestamp;

volatile bool flag = false;
Timestamp ts0;
Timestamp ts1;

void wait_and_write_timestamp(Timestamp *target) {
    while (!flag) {}
    target->tsc = __rdtsc();
    clock_gettime(CLOCK_MONOTONIC_RAW, &target->tp);
}

std::optional<std::tuple<unsigned long long, long>> test_sync() {
    flag = false;
    std::thread t0(wait_and_write_timestamp, &ts0);
    std::thread t1(wait_and_write_timestamp, &ts1);
    set_cpu_affinity(CPU_W0, t0);
    set_cpu_affinity(CPU_W1, t1);
    flag = true;
    t0.join();
    t1.join();

    if (ts0.tp.tv_sec != ts1.tp.tv_sec) return std::nullopt;

    return std::optional(std::tuple{MAX(ts0.tsc, ts1.tsc) - MIN(ts0.tsc, ts1.tsc), MAX(ts0.tp.tv_nsec, ts1.tp.tv_nsec) - MIN(ts0.tp.tv_nsec, ts1.tp.tv_nsec)});
}

int main() {
    /*
    auto res = test_sync();
    if (!res) return -1;
    const auto [tsc_diff, clock_diff] = res.value();
    std::cout << "TSC Diff: " << tsc_diff << std::endl;
    std::cout << "CLOCK_MONOTONIC_RAW Diff (ns): " << clock_diff << std::endl;
    */
    std::cout << "tsc diff,CLOCK_MONOTONIC_RAW diff" << std::endl;
    for (int i = 0; i < 10000; i++) {
        auto res = test_sync();
        if (!res.has_value()) continue;
        const auto [tsc_diff, clock_diff] = res.value();
        std::cout << tsc_diff << ", " << clock_diff << std::endl;
    }
}