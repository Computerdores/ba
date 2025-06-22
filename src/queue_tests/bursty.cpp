#include "queues/b_queue.h"
#include "queues/equeue.h"
#include "utils.h"

#define CPU_RX 2
#define CPU_TX 3

volatile bool start = false;

template <typename T>
void producer(T* queue, u64* start_times, u64* end_times, const usize count, const usize rate, const usize burst_size) {
    const u32 burst_wait_duration = (1'000'000'000 * burst_size) / rate;
    while (!start) {
    }
    auto next_time = get_timestamp();
    for (usize i = 0; i < count; i++) {
        auto start_tx = get_timestamp();
        while (!queue->enqueue(get_timestamp())) {
            start_tx = get_timestamp();
        }
        const auto end_tx = get_timestamp();

        start_times[i] = start_tx;
        end_times[i] = end_tx;

        // transmit `burst_size` elements before waiting
        if (i % burst_size == 0) {
            busy_wait_for(next_time);
            next_time += burst_wait_duration;
        }
    }
}

template <typename T>
void consumer(T* queue, u64* start_times, u64* end_times, const usize count, const usize rate) {
    const u32 wait_duration = 1'000'000'000 / rate;
    while (!start) {
    }
    auto next_time = get_timestamp();
    for (usize i = 0; i < count; i++) {
        auto rx_start = get_timestamp();
        while (!queue->dequeue()) {
            rx_start = get_timestamp();
        }
        const auto rx_end = get_timestamp();

        start_times[i] = rx_start;
        end_times[i] = rx_end;

        next_time += wait_duration;
        busy_wait_for(next_time);
    }
}

int main() {
    usize MSG_COUNT = 1'000'000;
    usize CONSUMER_RATE = 1'000'000;
    usize PRODUCER_RATE = 1'000'000;
    usize BURST_SIZE = 2048;

    auto tx_start = new u64[MSG_COUNT];
    auto tx_end = new u64[MSG_COUNT];
    auto rx_start = new u64[MSG_COUNT];
    auto rx_end = new u64[MSG_COUNT];

    queues::b_queue q(16384, 8192, 64, 50);

    std::thread tx(&producer<decltype(q)>, &q, tx_start, tx_end, MSG_COUNT, PRODUCER_RATE, BURST_SIZE);
    std::thread rx(&consumer<decltype(q)>, &q, rx_start, rx_end, MSG_COUNT, CONSUMER_RATE);

    set_cpu_affinity(CPU_RX, rx);
    set_cpu_affinity(CPU_TX, tx);

    start = true;

    tx.join();
    rx.join();

    std::cout << "TX_Start,TX_End,RX_Start,RX_End" << std::endl;
    for (usize i = 0; i < MSG_COUNT; i++) {
        std::cout << tx_start[i] << "," << tx_end[i] << "," << rx_start[i] << "," << rx_end[i] << std::endl;
    }

    delete tx_start;
    delete tx_end;
    delete rx_start;
    delete rx_end;
}
