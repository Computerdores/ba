#pragma once
#include "abstract.h"
#include "utils.h"

namespace waiter {

class ConstantWait final : public Abstract {
  public:
    template <typename P>
    explicit ConstantWait(const P& params) : _wait_time(NS_PER_S / params.rate) {}

    inline void start() override {}
    inline void wait() override { busy_wait(_wait_time); }

  private:
    usize _wait_time;
};

}  // namespace waiter
