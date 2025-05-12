#pragma once
#include <cstdint>
#include <thread>
#include <pthread.h>
#include <iostream>
#include <ctime>

#define u8  std::uint8_t
#define u16 std::uint16_t
#define u32 std::uint32_t
#define u64 std::uint64_t

#define MIN(A, B) (A > B ? B : A)
#define MAX(A, B) (A < B ? B : A)

inline u64 get_timestamp() {
    // gets a timestamp that is synchronised between cores (ns since unix epoch)
    // cross core desync should be <400ns in the vast majority of cases
    timespec ts;
    assert(clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0);
    return ts.tv_sec * 1000000000 + ts.tv_nsec; // breaks in 2554 CE
}

inline void set_cpu_affinity(const int cpu, std::thread &thread) {
    cpu_set_t cpuset;
    const pthread_t pthread = thread.native_handle();
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (const auto result = pthread_setaffinity_np(pthread, sizeof(cpu_set_t), &cpuset); result != 0) {
        std::cerr << "Failed to set CPU affinity to: " << cpu << ", error code: " << result << std::endl;
        exit(-1);
    }
}