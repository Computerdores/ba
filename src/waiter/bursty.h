#pragma once

#include "utils.h"
#include "waiter/abstract.h"

namespace waiter {

class Bursty final : public Abstract {
  public:
    template <typename P>
    explicit Bursty(const P& params)
        : _tx_wait(NS_PER_S * params.burst_size / params.producer_rate),
          _rx_wait(NS_PER_S / params.consumer_rate),
          _burst_size(params.burst_size) {}

    inline void tx_start() override { _tx_data.next_time = get_timestamp(); }
    inline void tx_wait() override {
        // transmit `burst_size` elements before waiting
        if (_tx_data.index % _burst_size == 0) {
            busy_wait_for(_tx_data.next_time);
            _tx_data.next_time += _tx_wait;
        }
        ++_tx_data.index;
    }

    inline void rx_start() override { _rx_data.next_time = get_timestamp(); }
    inline void rx_wait() override {
        _rx_data.next_time += _rx_wait;
        busy_wait_for(_rx_data.next_time);
    }

  private:
    usize _tx_wait;
    usize _rx_wait;
    usize _burst_size;

    CACHE_ALIGNED
    struct {
        usize next_time;
        usize index;
    } _tx_data = {};

    CACHE_ALIGNED
    struct {
        usize next_time;
    } _rx_data = {};
};

}  // namespace waiter
