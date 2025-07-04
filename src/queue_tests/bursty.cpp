
#include "bursty.h"

#include <thread>

#include "queues/b_queue.h"
#include "queues/equeue.h"
#include "queues/ff_queue.h"
#include "utils.h"

#define CPU_RX 2
#define CPU_TX 3

volatile bool start = false;

template <typename Q>
void producer(Q* queue, u64* start_times, u64* end_times, const usize count, const usize rate, const usize burst_size) {
    const u32 burst_wait_duration = (1'000'000'000 * burst_size) / rate;
    // warmup
    for (usize i = 0; i < 10'000; i++) {
        get_timestamp();
        while (!queue->enqueue(get_timestamp())) {
            get_timestamp();
        }
        const auto end = get_timestamp();

        start_times[i] = 0;
        end_times[i] = 0;

        busy_wait_for(end + 100);
    }
    // wait for start
    while (!start) {
    }
    // do test
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

template <typename Q>
void consumer(Q* queue, u64* start_times, u64* end_times, const usize count, const usize rate) {
    const u32 wait_duration = 1'000'000'000 / rate;
    // warmup
    for (usize i = 0; i < 10'000; i++) {
        get_timestamp();
        while (!queue->dequeue()) {
            get_timestamp();
        }
        const auto end = get_timestamp();

        start_times[i] = 0;
        end_times[i] = 0;

        busy_wait_for(end + 100);
    }
    // wait for start
    while (!start) {
    }
    // do test
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

template <typename Q>
void run_test(Q& q, test_parameters params) {
    auto tx_start = new u64[params.msg_count];
    auto tx_end = new u64[params.msg_count];
    auto rx_start = new u64[params.msg_count];
    auto rx_end = new u64[params.msg_count];

    std::thread tx(&producer<Q>, &q, tx_start, tx_end, params.msg_count, params.producer_rate, params.burst_size);
    std::thread rx(&consumer<Q>, &q, rx_start, rx_end, params.msg_count, params.consumer_rate);

    set_cpu_affinity(CPU_RX, rx);
    set_cpu_affinity(CPU_TX, tx);

    start = true;

    tx.join();
    rx.join();

    std::cout << "TX_Start,TX_End,RX_Start,RX_End" << std::endl;
    for (usize i = 0; i < params.msg_count; i++) {
        std::cout << tx_start[i] << "," << tx_end[i] << "," << rx_start[i] << "," << rx_end[i] << std::endl;
    }

    delete tx_start;
    delete tx_end;
    delete rx_start;
    delete rx_end;
}

int main() {
    test_parameters params = {};

    queues::b_queue bq(16384, 8192, 64, 50);
    queues::equeue eq(4096, 16384, 50);
    queues::ff_queue ffq(1024, 16);

    run_test(ffq, params);
}
