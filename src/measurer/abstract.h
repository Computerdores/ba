#pragma once
#include <concepts>

namespace measurer {

class Abstract {
  public:
    virtual ~Abstract() = default;

    inline virtual void print_results() = 0;

    inline virtual void pre_rx() = 0;
    inline virtual void post_rx() = 0;

    inline virtual void pre_tx() = 0;
    inline virtual void post_tx() = 0;
};

template <typename T>
concept IsMeasurer = std::derived_from<T, Abstract>;

}  // namespace measurer
