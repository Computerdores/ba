#pragma once
#include "abstract.h"
#include "utils.h"

namespace queues {

template <typename T = u64>
class lamport final : public Queue<T> {
  public:
    explicit lamport(const usize size) : _SIZE(size) { _buffer = new T[size]; }

    bool enqueue(T item) override {
        if (_next(_head) == _tail) {
            // lamport paper specifies busy wait, but we require non-blocking fail
            // (also supported by EQueue and FastForward papers)
            return false;
        }
        _buffer[_head] = item;
        _head = _next(_head);
        return true;
    }

    std::optional<T> dequeue() override {
        if (_head == _tail) {
            // as with enqueue, lamport paper specifies busy wait, but we require non-blocking fail
            return false;
        }
        auto out = _buffer[_tail];
        _head = _next(_head);
        return out;
    }

  private:
    CACHE_ALIGNED usize _head = 0;
    CACHE_ALIGNED usize _tail = 0;

    CACHE_ALIGNED
    T *_buffer;
    usize _SIZE;

    usize _next(const usize index) const { return (index + 1) % _SIZE; }
};

}  // namespace queues
