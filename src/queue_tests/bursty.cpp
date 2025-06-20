#include "queues/b_queue.h"
#include "queues/equeue.h"
#include "utils.h"

#define CPU_RX 2
#define CPU_TX 3

volatile bool start = false;

template <typename T>
void producer(T *queue, const usize count, const usize rate, const usize burst_size) {
    const u32 burst_wait_duration = (1'000'000'000 * burst_size) / rate;
    while (!start) {
    }
    auto next_time = get_timestamp();
    for (usize i = 0; i < count; i++) {
        while (!queue->enqueue(get_timestamp())) {
        }
        if (i % burst_size == 0) {
            busy_wait_for(next_time);
            next_time += burst_wait_duration;
        }
    }
}

template <typename T>
void consumer(T *queue, const usize count, const usize rate) {
    const u32 wait_duration = 1'000'000'000 / rate;
    while (!start) {
    }
    auto next_time = get_timestamp();
    for (usize i = 0; i < count; i++) {
        while (!queue->dequeue()) {
        }
        next_time += wait_duration;
        busy_wait_for(next_time);
    }
}

int main() {
    usize MSG_COUNT = 1'000'000;
    usize CONSUMER_RATE = 1'000'000;
    usize PRODUCER_RATE = 1'000'000;
    usize BURST_SIZE = 2048;

    queues::b_queue q(16384, 8192, 64, 50);

    std::thread tx(&producer<decltype(q)>, &q, MSG_COUNT, PRODUCER_RATE, BURST_SIZE);
    std::thread rx(&consumer<decltype(q)>, &q, MSG_COUNT, CONSUMER_RATE);

    set_cpu_affinity(CPU_RX, rx);
    set_cpu_affinity(CPU_TX, tx);

    auto start_time = get_timestamp();
    start = true;

    tx.join();
    rx.join();
    std::cout << "Done after: " << get_timestamp() - start_time << std::endl;
}
