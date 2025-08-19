#pragma once

#include <random>

#include "utils.h"
#include "waiter/abstract.h"

namespace waiter {

template <bool JITTER = false, s64 JITTER_RANGE = 200>
class Bursty final : public Abstract {
  public:
    template <typename P>
    explicit Bursty(const P& params)
        : _wait_time(NS_PER_S * params.burst_size / params.rate), _burst_size(params.burst_size) {
        if constexpr (JITTER) {
            _generator = std::default_random_engine(1);
        }
    }

    inline void start() override { _next_time = get_timestamp(); }
    inline void wait() override {
        // transmit `burst_size` elements before waiting
        if (_index % _burst_size == 0) {
            _next_time += _wait_time;
            if constexpr (JITTER) {
                const s64 jitter = static_cast<s64>(_generator() % JITTER_RANGE) - (JITTER_RANGE >> 1);
                busy_wait_for(_next_time + jitter);
            } else {
                busy_wait_for(_next_time);
            }
        }
        ++_index;
    }

  private:
    usize _wait_time;
    usize _burst_size;
    usize _next_time = 0;
    usize _index = 0;
    std::conditional_t<JITTER, std::default_random_engine, std::monostate> _generator;
};

}  // namespace waiter
