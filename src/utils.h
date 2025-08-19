#pragma once
#include <pthread.h>
#include <x86intrin.h>

#include <cassert>
#include <ctime>
#include <iostream>
#include <thread>

#define NS_PER_S 1'000'000'000

#define u8 std::uint8_t
#define u16 std::uint16_t
#define u32 std::uint32_t
#define u64 std::uint64_t
#define usize std::size_t

#define s8 std::int8_t
#define s16 std::int16_t
#define s32 std::int32_t
#define s64 std::int64_t

#define MIN(A, B) (A > B ? B : A)
#define MAX(A, B) (A < B ? B : A)

#define CACHE_LINE_SIZE std::hardware_destructive_interference_size
#define CACHE_ALIGNED alignas(CACHE_LINE_SIZE)

/**
 * @brief Gets the current timestamp relative to the unix epoch in nanoseconds.
 *        This timestamp is synchronised between cores (desync should be <400ns in vast the majority of cases).
 * @return The timestamp in nanoseconds.
 */
inline u64 get_clock_timestamp() {
    // overview of clock sources: http://btorpey.github.io/blog/2014/02/18/clock-sources-in-linux/
    timespec ts {};
    assert(clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0);
    return ts.tv_sec * 1'000'000'000 + ts.tv_nsec;  // breaks in 2554 CE
}

#define TSC_FAC 10
#define TSC_DIV 28  // TODO: determine this dynamically; current value is for bp-flugzeug machine
inline u64 get_tsc_timestamp() { return (__rdtsc() * TSC_FAC) / TSC_DIV; }

#define get_timestamp get_tsc_timestamp

/**
 * Busy waits until the target time has been reached.
 * @param target_time A point in time that (presumably) lies in the future.
 */
inline void busy_wait_for(const u64 target_time) { while (get_timestamp() < target_time); }

/**
 * @brief Busy waits for a given duration using @ref get_timestamp.
 * @param duration Wait duration in nanoseconds.
 */
inline void busy_wait(const u32 duration) { busy_wait_for(get_timestamp() + duration); }

/**
 * @brief Sets the cpu affinity of a thread.
 * @param cpu Index of the cpu to set the affinity to.
 * @param thread Thread object to set the cpu affinity for.
 */
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
