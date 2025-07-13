#pragma once

namespace waiter {

class Abstract {
  public:
    virtual ~Abstract() = default;

    inline virtual void tx_start() = 0;
    inline virtual void tx_wait() = 0;

    inline virtual void rx_start() = 0;
    inline virtual void rx_wait() = 0;
};

template <typename T>
concept IsWaiter = std::derived_from<T, Abstract>;

}  // namespace waiter
