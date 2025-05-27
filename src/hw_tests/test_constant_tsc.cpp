#include <x86intrin.h>
#include "../utils.h"

int main() {
    const u64 ITERS = 100000;
    u64 sum = 0;
    for (int i = 0; i < ITERS; i++) {
        u64 t0 = __rdtsc();
        u64 t1 = __rdtsc();
        u64 diff = t1 - t0;
        std::cout << diff << std::endl;
        sum += diff;
    }
    std::cout << sum / static_cast<double>(ITERS) << std::endl;
}