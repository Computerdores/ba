#pragma once
#include <concepts>
#include <optional>

template <typename T>
class Queue {
  public:
    using Item = T;

    virtual bool enqueue(T item) = 0;

    virtual std::optional<T> dequeue() = 0;

    virtual ~Queue() = default;
};

template <typename Q, typename T>
concept IsQueueWith = std::derived_from<Q, Queue<T>>;

template <typename Q>
concept IsQueue = IsQueueWith<Q, typename Q::Item>;
