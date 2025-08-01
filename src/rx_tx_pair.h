#pragma once
#include <concepts>

#include "utils.h"

template <typename _RX, typename _TX>
class RXTXPair {
  public:
    using RX = _RX;
    using TX = _TX;

    template <typename P>
    explicit RXTXPair(P& params) : rx(params.rx), tx(params.tx) {}

    CACHE_ALIGNED
    RX rx;

    CACHE_ALIGNED
    TX tx;
};

template <typename T>
class SimplePair {
  public:
    using RX = T;
    using TX = RX;

    template <typename... Args>
    explicit SimplePair(Args&&... args) : rx(std::forward<Args>(args)...), tx(std::forward<Args>(args)...) {}

    CACHE_ALIGNED
    RX rx;

    CACHE_ALIGNED
    TX tx;
};

template <typename T>
concept IsRXTXPair =
    std::derived_from<T, RXTXPair<typename T::RX, typename T::TX>> || std::derived_from<T, SimplePair<typename T::RX>>;
