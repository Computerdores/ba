#include "ff_queue.hpp"
#include <thread>
#include "utils.h"

// SO on cpusets: https://stackoverflow.com/a/9079117
#define CPU_RX 3
#define CPU_TX 4

#define MSG_COUNT 1024

ff_queue *channel;
volatile bool start = false;

struct {
    u64     tx_times[MSG_COUNT];
    size_t  tx_misses;

    u8      OFFSET[CACHE_LINE_SIZE];

    u64     rx_times[MSG_COUNT];
    size_t  rx_misses;
} state = {};


void sender() {
    while (!start) {}
    size_t count = 0;
    while (count < MSG_COUNT) {
        u64 *slot = static_cast<u64 *>(channel->enqueue_prepare(sizeof(u64)));
        if (!slot) {
            state.tx_misses++;
            continue;
        }
        *slot = 42;
        channel->enqueue_commit();
        state.tx_times[count] = get_timestamp();
        count++;
    }
}

void receiver() {
    while (!start) {}
    size_t count = 0;
    while (count < MSG_COUNT) {
        u64 *slot = static_cast<u64 *>(channel->dequeue_prepare());
        if (!slot) {
            state.rx_misses++;
            continue;
        }
        assert(*slot == 42);
        channel->dequeue_commit();
        state.rx_times[count] = get_timestamp();
        count++;
    }
}

void run_test() {
    std::thread tx(sender);
    std::thread rx(receiver);

    set_cpu_affinity(CPU_RX, rx);
    set_cpu_affinity(CPU_TX, tx);

    start = true;

    tx.join();
    rx.join();
}

void reset_test() {
    start = false;
    if (channel) {
        delete channel;
    }
    channel = new ff_queue(1024, 10);
}

int main() {
    std::cout << "RX,TX" << std::endl;

    reset_test();
    run_test();

    for (int i = 0; i < MSG_COUNT; i++) {
        std::cout << state.rx_times[i] << "," << state.tx_times[i] << std::endl;
    }

    // TODO: store time_delta, ts_delta, and state for analysis
}