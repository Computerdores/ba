#include <x86intrin.h>

#include "utils.h"

int main() {
    constexpr u64 ITERS = 1000000;

    u64 sum = 0;

    for (int i = 0; i < ITERS; i++) {
        const u64 t0 = __rdtsc();
        const u64 t1 = __rdtsc();
        const u64 diff = t1 - t0;
        std::cout << diff << std::endl;
        sum += diff;
    }
    std::cout << static_cast<double>(sum) / static_cast<double>(ITERS) << std::endl;
}
