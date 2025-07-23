#pragma once
#include <concepts>

template <typename _RX, typename _TX>
class RXTXPair {
  public:
    using RX = _RX;
    using TX = _TX;

    template <typename... Args>
    explicit RXTXPair(Args&&... args) : rx(std::forward<Args>(args)...), tx(std::forward<Args>(args)...) {}

    CACHE_ALIGNED
    RX rx;

    CACHE_ALIGNED
    TX tx;
};

template <typename T>
using SimplePair = RXTXPair<T, T>;

template <typename T>
concept IsRXTXPair = std::derived_from<T, RXTXPair<typename T::RX, typename T::TX>>;
