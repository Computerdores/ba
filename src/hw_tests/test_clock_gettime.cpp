#include <ctime>
#include <iostream>

#include "utils.h"

int main() {
    constexpr u64 ITERS = 10000;
    timespec ts0 = {};
    timespec ts1 = {};

    u64 sum = 0;

    for (int i = 0; i < ITERS; i++) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts0);
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts1);

        if (ts0.tv_sec != ts1.tv_sec) continue;

        const u64 diff = ts1.tv_nsec - ts0.tv_nsec;
        std::cout << diff << std::endl;
        sum += diff;
    }

    std::cout << static_cast<double>(sum) / static_cast<double>(ITERS) << std::endl;
}
