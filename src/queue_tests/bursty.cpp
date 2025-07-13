
#include "bursty.h"

#include <thread>

#include "queues/b_queue.h"
#include "queues/equeue.h"
#include "queues/ff_queue.h"
#include "queues/mc_ring_buffer.h"
#include "utils.h"

#define CPU_RX 2
#define CPU_TX 3

volatile bool start = false;

class Test {
  public:
    explicit Test(const test_parameters& params)
        : _msg_count(params.msg_count),
          _tx_start(new u64[params.msg_count]),
          _tx_end(new u64[params.msg_count]),
          _rx_start(new u64[params.msg_count]),
          _rx_end(new u64[params.msg_count]) {}

    ~Test() {
        delete _tx_start;
        delete _tx_end;
        delete _rx_start;
        delete _rx_end;
    }

    void print_results() const {
        std::cout << "TX_Start,TX_End,RX_Start,RX_End" << std::endl;
        for (usize i = 0; i < _msg_count; i++) {
            std::cout << _tx_start[i] << "," << _tx_end[i] << "," << _rx_start[i] << "," << _rx_end[i] << std::endl;
        }
    }

    inline void pre_rx() { _rx_data.pre_val = get_timestamp(); }
    inline void post_rx() {
        const auto post_val = get_timestamp();
        _rx_start[_rx_data.index] = _rx_data.pre_val;
        _rx_end[_rx_data.index] = post_val;
        _rx_data.index++;
    }

    inline void pre_tx() { _tx_data.pre_val = get_timestamp(); }
    inline void post_tx() {
        const auto post_val = get_timestamp();
        _tx_start[_tx_data.index] = _tx_data.pre_val;
        _tx_end[_tx_data.index] = post_val;
        _tx_data.index++;
    }

  private:
    usize _msg_count;
    u64* _tx_start;
    u64* _tx_end;
    u64* _rx_start;
    u64* _rx_end;

    CACHE_ALIGNED
    struct {
        usize index;
        usize pre_val;
    } _rx_data = {};

    CACHE_ALIGNED
    struct {
        usize index;
        usize pre_val;
    } _tx_data = {};
};

template <IsQueueWith<u64> Q>
class Runner {
  public:
    explicit Runner(Q* queue, test_parameters& params) : _queue(queue), _test(Test(params)), _params(params) {}

    void run() {
        std::thread tx([&] { producer(); });
        std::thread rx([&] { consumer(); });

        set_cpu_affinity(CPU_RX, rx);
        set_cpu_affinity(CPU_TX, tx);

        start = true;

        tx.join();
        rx.join();

        _test.print_results();
    }

  private:
    Q* _queue;
    Test _test;
    test_parameters& _params;

    void producer() {
        const u32 burst_wait_duration = NS_PER_S * _params.burst_size / _params.producer_rate;
        // wait for start
        while (!start) {
        }
        // do test
        auto next_time = get_timestamp();
        for (usize i = 0; i < _params.msg_count; i++) {
            _test.pre_tx();
            while (!_queue->enqueue(get_timestamp())) {
                _test.pre_tx();
            }
            _test.post_tx();

            // transmit `burst_size` elements before waiting
            if (i % _params.burst_size == 0) {
                busy_wait_for(next_time);
                next_time += burst_wait_duration;
            }
        }
    }

    void consumer() {
        const u32 wait_duration = 1'000'000'000 / _params.consumer_rate;
        // wait for start
        while (!start) {
        }
        // do test
        auto next_time = get_timestamp();
        for (usize i = 0; i < _params.msg_count; i++) {
            _test.pre_rx();
            while (!_queue->dequeue()) {
                _test.pre_rx();
            }
            _test.post_rx();

            next_time += wait_duration;
            busy_wait_for(next_time);
        }
    }
};

int main() {
    test_parameters params = {};

    queues::b_queue bq(16384, 8192, 64, 50);
    queues::equeue eq(4096, 16384, 50);
    queues::ff_queue ffq(1024, 16);
    queues::mc_ring_buffer mcrb(16384, 5000);

    Runner r(&mcrb, params);
    r.run();
}
