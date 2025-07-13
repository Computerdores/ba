#pragma once
#include "abstract.h"
#include "utils.h"

namespace waiter {

class Continuous final : public Abstract {
  public:
    template <typename P>
    explicit Continuous(const P& params)
        : _tx_wait(NS_PER_S / params.producer_rate), _rx_wait(NS_PER_S / params.consumer_rate) {}

    inline void tx_start() override { _tx_next_time = get_timestamp(); }
    inline void tx_wait() override {
        _tx_next_time += _tx_wait;
        busy_wait_for(_tx_next_time);
    }

    inline void rx_start() override { _rx_next_time = get_timestamp(); }
    inline void rx_wait() override {
        _rx_next_time += _rx_wait;
        busy_wait_for(_rx_next_time);
    }

  private:
    usize _tx_wait;
    usize _rx_wait;

    CACHE_ALIGNED
    usize _tx_next_time = 0;

    CACHE_ALIGNED
    usize _rx_next_time = 0;
};

}  // namespace waiter
