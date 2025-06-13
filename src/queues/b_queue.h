#pragma once
#include <optional>

#include "queues/abstract.h"
#include "utils.h"

namespace queues {

template <typename T = u64>
class b_queue final : public Queue<T> {
    // TODO: investigate whether the buffer being optionals has a significant performance impact,
    //       because that could introduce bias
  public:
    explicit b_queue(const usize size, const usize batch_size, const u32 wait_time)
        : _SIZE(size), _BATCH_SIZE(batch_size), _WAIT_TIME(wait_time), _buffer(new std::optional<T>[size]) {
        assert(batch_size < size);
    }

    bool enqueue(T item) override {
        if (_head == _batch_head) {
            auto new_batch_head = mod(_head + _BATCH_SIZE);
            if (_buffer[new_batch_head]) return false;
            _batch_head = new_batch_head;
        }
        _buffer[_head] = item;
        _head = mod(_head + 1);
        return true;
    }

    std::optional<T> dequeue() override {
        if (_tail == _batch_tail) {
            if (!_backtrack_deq()) return std::nullopt;
        }
        auto out = _buffer[_tail];
        _buffer[_tail] = std::nullopt;
        _tail = mod(_tail + 1);
        return out;
    }

    ~b_queue() override { delete[] _buffer; }

  private:
    usize _SIZE;
    usize _BATCH_SIZE;
    u32 _WAIT_TIME;
    std::optional<T> *_buffer;

    CACHE_ALIGNED usize _head = 0;
    CACHE_ALIGNED usize _batch_head = 0;

    CACHE_ALIGNED usize _tail = 0;
    CACHE_ALIGNED usize _batch_tail = 0;

    bool _backtrack_deq() {
        auto batch_size = _BATCH_SIZE;
        _batch_tail = mod(_tail + batch_size - 1);

        while (!_buffer[_batch_tail]) {
            busy_wait(_WAIT_TIME);
            if (batch_size > 1) {
                batch_size = batch_size >> 1;
                _batch_tail = mod(_tail + batch_size - 1);
            } else
                return false;
        }
        return true;
    }

    usize mod(const usize index) const { return index % _SIZE; }
};
}  // namespace queues
