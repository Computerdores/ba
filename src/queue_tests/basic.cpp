#include <cmath>
#include <thread>
#include <utility>

#include "generic_factory.h"
#include "queues/b_queue.h"
#include "queues/equeue.h"
#include "queues/ff_queue.h"
#include "utils.h"

// SO on cpusets: https://stackoverflow.com/a/9079117
#define CPU_RX 2
#define CPU_TX 3

#define MSG_COUNT 1024

#define MIN_WAIT 1000
#define MAX_WAIT 1000000
#define WAIT_GRANULARITY 128

template <typename Factory>
    requires IsQueue<typename Factory::Object>
class TestRunner {
  public:
    using Queue = typename Factory::Object;

    explicit TestRunner(Factory factory) : _queue_factory(std::move(factory)) {}

    void run(u32 wait_time) {
        reset();

        std::thread tx(&TestRunner::sender, this, wait_time);
        std::thread rx(&TestRunner::receiver, this, wait_time);

        set_cpu_affinity(CPU_RX, rx);
        set_cpu_affinity(CPU_TX, tx);

        start = true;

        tx.join();
        rx.join();

        for (int i = 0; i < MSG_COUNT; i++) {
            std::cout << rx_start[i] << "," << rx_end[i] << "," << tx_start[i] << "," << tx_end[i] << "," << wait_time
                      << std::endl;
        }
    }

  private:
    Factory _queue_factory;
    Queue *channel = nullptr;

    volatile bool start = false;

    u64 tx_start[MSG_COUNT] = {};
    u64 tx_end[MSG_COUNT] = {};

    u8 OFFSET[CACHE_LINE_SIZE] = {};

    u64 rx_start[MSG_COUNT] = {};
    u64 rx_end[MSG_COUNT] = {};

    void reset() {
        start = false;
        if (channel) {  // NOLINT(*-delete-null-pointer)
            delete channel;
        }
        channel = _queue_factory.build();
    }

    void sender(const u32 wait_time) {
        while (!start) {
        }
        size_t count = 0;
        while (count < MSG_COUNT) {
            busy_wait(wait_time);
            tx_start[count] = get_timestamp();
            auto success = channel->enqueue(42);
            tx_end[count] = get_timestamp();  // TODO: check if making this conditional on success is faster
            count += success;
        }
    }

    void receiver(const u32 wait_time) {
        while (!start) {
        }
        size_t count = 0;
        while (count < MSG_COUNT) {
            busy_wait(wait_time);
            rx_start[count] = get_timestamp();
            auto res = channel->dequeue();
            assert(res.has_value() == (res == std::optional(42)));
            rx_end[count] = get_timestamp();  // TODO: check if making this conditional on success is faster
            count += res.has_value();
        }
    }
};

#define EQueue false

int main() {
    std::cout << "RX_Start,RX_End,TX_Start,TX_End,Wait_Time" << std::endl;

    TestRunner eq_runner {GenericFactory<queues::equeue, u64, u64>(16, 32)};
    TestRunner bq_runner {
        GenericFactory<queues::b_queue<>, usize, usize, u32>(128, 64, 50)};  // TODO: what is a sensible wait time?
    TestRunner ffq_runner {GenericFactory<queues::ff_queue<>, u64, u64>(1024, 10)};

    auto runner = bq_runner;

    for (int i = 0; i < WAIT_GRANULARITY; i++) {
        const double wait_time = MIN_WAIT + (MAX_WAIT - MIN_WAIT) * (static_cast<double>(i) / (WAIT_GRANULARITY - 1));
        runner.run(floor(wait_time));
        std::cerr << "Test " << i << " done." << std::endl;
    }
}
