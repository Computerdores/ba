#pragma once
#include <thread>

#include "measurer/abstract.h"
#include "queues/abstract.h"
#include "utils.h"
#include "waiter/abstract.h"

using measurer::IsMeasurer;
using queues::IsQueueWith;
using waiter::IsWaiter;

template <IsQueueWith<u64> Q, IsMeasurer M, IsWaiter W, int CPU_RX = 2, int CPU_TX = 3>
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

        _measurer.print_results();
    }

  private:
    Q* _queue;
    M _measurer;
    W _waiter;
    usize _msg_count;
    volatile bool _start = false;

    void producer() {
        // wait for start
        while (!_start) {
        }
        // do test
        _waiter.tx_start();
        for (usize i = 0; i < _msg_count; i++) {
            _measurer.pre_tx();
            while (!_queue->enqueue(get_timestamp())) {
                _measurer.pre_tx();
            }
            _measurer.post_tx();
            _waiter.tx_wait();
        }
    }

    void consumer() {
        // wait for start
        while (!_start) {
        }
        // do test
        _waiter.rx_start();
        for (usize i = 0; i < _msg_count; i++) {
            _measurer.pre_rx();
            while (!_queue->dequeue()) {
                _measurer.pre_rx();
            }
            _measurer.post_rx();
            _waiter.rx_wait();
        }
    }
};
