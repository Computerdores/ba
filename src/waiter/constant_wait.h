#pragma once
#include "abstract.h"
#include "utils.h"

namespace waiter {

class ConstantWait final : public Abstract {
  public:
    template <typename P>
    explicit ConstantWait(const P& params)
        : _tx_wait(NS_PER_S / params.producer_rate), _rx_wait(NS_PER_S / params.consumer_rate) {}

    inline void tx_start() override {}
    inline void tx_wait() override {
        busy_wait(_tx_wait);
    }

    inline void rx_start() override {}
    inline void rx_wait() override {
        busy_wait(_rx_wait);
    }

  private:
    usize _tx_wait;
    usize _rx_wait;
};

}  // namespace waiter
