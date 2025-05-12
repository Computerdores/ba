#include "ff_queue.hpp"
#include <thread>
#include "utils.h"


#define MSG_COUNT 1024

auto channel = ff_queue(1024, 10);

struct {
    u64     tx_start;
    u64     tx_end;
    u64     tx_times[MSG_COUNT];
    size_t  tx_misses;

    u8      OFFSET[CACHE_LINE_SIZE];

    u64     rx_start;
    u64     rx_end;
    u64     rx_times[MSG_COUNT];
    size_t  rx_misses;
} state = {};


void sender() {
    state.tx_start = get_timestamp();
    size_t count = 0;
    while (count < MSG_COUNT) {
        u64 *slot = static_cast<std::uint64_t *>(channel.enqueue_prepare(sizeof(u64)));
        if (!slot) {
            state.tx_misses++;
            continue;
        }
        *slot = 42;
        channel.enqueue_commit();
        state.tx_times[count] = get_timestamp();
        count++;
    }
    state.tx_end = get_timestamp();
}

void receiver() {
    state.rx_start = get_timestamp();
    size_t count = 0;
    while (count < MSG_COUNT) {
        u64 *slot = static_cast<std::uint64_t *>(channel.dequeue_prepare());
        if (!slot) {
            state.rx_misses++;
            continue;
        }
        assert(*slot == 42);
        channel.dequeue_commit();
        state.rx_times[count] = get_timestamp();
        count++;
    }
    state.rx_end = get_timestamp();
}

int main() {
    const auto _start_time = std::chrono::high_resolution_clock::now();
    const auto _start_ts = get_timestamp();

    std::thread t1(sender);
    std::thread t2(receiver);
    t1.join();
    t2.join();

    const auto _end_time = std::chrono::high_resolution_clock::now();
    const auto _end_ts = get_timestamp();

    auto time_delta = std::chrono::duration_cast<std::chrono::nanoseconds>(_end_time - _start_time).count();
    auto ts_delta = _end_ts - _start_ts;

    // TODO: store time_delta, ts_delta, and state for analysis
    // TODO: move all previous logic to parametrised function for varied testing
}