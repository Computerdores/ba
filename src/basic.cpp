#include <cmath>
#include <thread>

#include "queues/ff_queue.h"
#include "utils.h"

// SO on cpusets: https://stackoverflow.com/a/9079117
#define CPU_RX 3
#define CPU_TX 4

#define MSG_COUNT 1024

#define MIN_WAIT 1000
#define MAX_WAIT 1000000
#define WAIT_GRANULARITY 128

ff_queue *channel;
volatile bool start = false;

struct {
    u64 tx_start[MSG_COUNT];
    u64 tx_end[MSG_COUNT];
    size_t tx_misses;

    u8 OFFSET[CACHE_LINE_SIZE];

    u64 rx_start[MSG_COUNT];
    u64 rx_end[MSG_COUNT];
    size_t rx_misses;
} state = {};

void sender(u32 wait_time) {
    while (!start) {
    }
    size_t count = 0;
    while (count < MSG_COUNT) {
        nsleep(wait_time);
        state.tx_start[count] = get_timestamp();
        u64 *slot = static_cast<u64 *>(channel->enqueue_prepare(sizeof(u64)));
        if (!slot) {
            state.tx_misses++;
            continue;
        }
        *slot = 42;
        channel->enqueue_commit();
        state.tx_end[count] = get_timestamp();
        count++;
    }
}

void receiver(u32 wait_time) {
    while (!start) {
    }
    size_t count = 0;
    while (count < MSG_COUNT) {
        nsleep(wait_time);
        state.rx_start[count] = get_timestamp();
        u64 *slot = static_cast<u64 *>(channel->dequeue_prepare());
        if (!slot) {
            state.rx_misses++;
            continue;
        }
        assert(*slot == 42);
        channel->dequeue_commit();
        state.rx_end[count] = get_timestamp();
        count++;
    }
}

void reset_test() {
    start = false;
    if (channel) {
        delete channel;
    }
    channel = new ff_queue(1024, 10);
}

void run_test(u32 wait_time) {
    reset_test();
    std::thread tx(sender, wait_time);
    std::thread rx(receiver, wait_time);

    set_cpu_affinity(CPU_RX, rx);
    set_cpu_affinity(CPU_TX, tx);

    start = true;

    tx.join();
    rx.join();

    for (int i = 0; i < MSG_COUNT; i++) {
        std::cout << state.rx_start[i] << "," << state.rx_end[i] << "," << state.tx_start[i] << "," << state.tx_end[i]
                  << "," << wait_time << std::endl;
    }
}

int main() {
    std::cout << "RX_Start,RX_End,TX_Start,TX_End,Wait_Time" << std::endl;

    for (int i = 0; i < WAIT_GRANULARITY; i++) {
        double wait_time = MIN_WAIT + (MAX_WAIT - MIN_WAIT) * (static_cast<double>(i) / (WAIT_GRANULARITY - 1));
        run_test(floor(wait_time));
    }
}
