#pragma once
#include <concepts>

#include "rx_tx_pair.h"
#include "utils.h"

namespace measurer {

class Abstract {
  public:
    virtual ~Abstract() = default;

    inline virtual std::string format_header(std::string prefix) = 0;
    inline virtual std::string row_to_string(usize index) = 0;

    inline virtual void pre() = 0;
    inline virtual void post() = 0;
};

template <typename T>
concept IsMeasurer = std::derived_from<T, Abstract>;

template <typename T>
concept IsMeasurerPair = IsRXTXPair<T> && IsMeasurer<typename T::RX> && IsMeasurer<typename T::TX>;

}  // namespace measurer
