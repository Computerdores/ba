#pragma once

#include "utils.h"
#include "waiter/abstract.h"

namespace waiter {

class Bursty final : public Abstract {
  public:
    template <typename P>
    explicit Bursty(const P& params)
        : _wait_time(NS_PER_S * params.burst_size / params.producer_rate), _burst_size(params.burst_size) {}

    inline void start() override { _next_time = get_timestamp(); }
    inline void wait() override {
        // transmit `burst_size` elements before waiting
        if (_index % _burst_size == 0) {
            busy_wait_for(_next_time);
            _next_time += _wait_time;
        }
        ++_index;
    }

  private:
    usize _wait_time;
    usize _burst_size;
    usize _next_time = 0;
    usize _index = 0;
};

}  // namespace waiter
