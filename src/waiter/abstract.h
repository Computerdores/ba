#pragma once
#include "rx_tx_pair.h"

namespace waiter {

class Abstract {
  public:
    virtual ~Abstract() = default;

    inline virtual void start() = 0;
    inline virtual void wait() = 0;
};

template <typename T>
concept IsWaiter = std::derived_from<T, Abstract>;

template <typename T>
concept IsWaiterPair = IsRXTXPair<T> && IsWaiter<typename T::RX> && IsWaiter<typename T::RX>;

}  // namespace waiter
