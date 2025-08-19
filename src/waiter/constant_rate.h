#pragma once
#include <random>

#include "abstract.h"
#include "utils.h"

namespace waiter {

template <bool JITTER = false, u64 JITTER_RANGE = 200>
class ConstantRate final : public Abstract {
  public:
    template <typename P>
    explicit ConstantRate(const P& params) : _wait_time(NS_PER_S / params.rate) {
        if constexpr (JITTER) {
            _generator = std::default_random_engine(1);
        }
    }

    inline void start() override { _next_time = get_timestamp(); }
    inline void wait() override {
        _next_time += _wait_time;
        if constexpr (JITTER) {
            const s64 jitter = static_cast<s64>(_generator() % JITTER_RANGE) - (JITTER_RANGE >> 1);
            busy_wait_for(_next_time + jitter);
        } else {
            busy_wait_for(_next_time);
        }
    }

  private:
    usize _wait_time;
    usize _next_time = 0;
    std::conditional_t<JITTER, std::default_random_engine, std::monostate> _generator;
};

}  // namespace waiter
