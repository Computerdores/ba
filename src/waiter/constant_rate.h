#pragma once
#include "abstract.h"
#include "utils.h"

namespace waiter {

class ConstantRate final : public Abstract {
  public:
    template <typename P>
    explicit ConstantRate(const P& params) : _wait_time(NS_PER_S / params.rate) {}

    inline void start() override { _next_time = get_timestamp(); }
    inline void wait() override {
        _next_time += _wait_time;
        busy_wait_for(_next_time);
    }

  private:
    usize _wait_time;
    usize _next_time = 0;
};

}  // namespace waiter
