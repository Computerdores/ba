#pragma once
#include <thread>

#include "measurer/abstract.h"
#include "queues/abstract.h"
#include "utils.h"
#include "waiter/abstract.h"

using measurer::IsMeasurerPair;
using queues::IsQueueWith;
using waiter::IsWaiterPair;

template <IsQueueWith<u64> Q, IsMeasurerPair M, IsWaiterPair W, bool MEASURE_FAILED = false, int CPU_RX = 2,
          int CPU_TX = 3>
class Runner {
  public:
    template <typename P>
    explicit Runner(Q* queue, const P& params)
        : _queue(queue), _measurer(M(params.msg_count)), _waiter(W(params)), _msg_count(params.msg_count) {}

    void run() {
        std::thread tx([&] { producer(); });
        std::thread rx([&] { consumer(); });

        set_cpu_affinity(CPU_RX, rx);
        set_cpu_affinity(CPU_TX, tx);

        _start = true;

        tx.join();
        rx.join();
    }

    void write_results(std::ostream* out) {
        *out << _measurer.tx.format_header("TX") << "," << _measurer.rx.format_header("RX") << std::endl;
        for (usize i = 0; i < _msg_count; i++) {
            *out << _measurer.tx.row_to_string(i) << "," << _measurer.rx.row_to_string(i) << std::endl;
        }
    }

  private:
    Q* _queue;
    M _measurer;
    W _waiter;
    usize _msg_count;
    volatile bool _start = false;
    CACHE_ALIGNED u64 _data_sink;

    void producer() {
        // wait for start
        while (!_start) {
        }
        // do test
        _waiter.tx.start();
        for (usize i = 0; i < _msg_count; i++) {
            _measurer.tx.pre();
            auto data = get_timestamp();  // arbitrary data to enqueue
            while (!_queue->enqueue(data)) {
                if constexpr (!MEASURE_FAILED) _measurer.tx.pre();
            }
            _measurer.tx.post();
            _waiter.tx.wait();
        }
    }

    void consumer() {
        // wait for start
        while (!_start) {
        }
        // do test
        _waiter.rx.start();
        for (usize i = 0; i < _msg_count; i++) {
            _measurer.rx.pre();
            std::optional<u64> data = std::nullopt;
            while (!((data = _queue->dequeue()))) {
                if constexpr (!MEASURE_FAILED) _measurer.rx.pre();
            }
            _data_sink += *data;  // do something with the data to force data retrieval from queue
            _measurer.rx.post();
            _waiter.rx.wait();
        }
    }
};
