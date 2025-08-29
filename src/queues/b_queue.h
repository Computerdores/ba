#pragma once
#include <optional>

#include "queues/abstract.h"
#include "utils.h"

namespace queues {

/**
 * State of this implementation:
 * - complete implementation of B-Queue with self-adaptive backtracking
 * - enqueue and dequeue differ from the paper in order to fix two bugs
 */
template <typename T = u64>
class b_queue final : public Queue<T> {
    // TODO: investigate whether the buffer being optionals has a significant performance impact,
    //       because that could introduce bias
  public:
    explicit b_queue(const usize size, const usize batch_size, const usize batch_increment, const u32 wait_time)
        : _SIZE(size),
          _BATCH_SIZE(batch_size),
          _BATCH_INCREMENT(batch_increment),
          _WAIT_TIME(wait_time),
          _buffer(new std::optional<T>[size]),
          _batch_history(batch_size) {
        assert(batch_size < size);
    }

    bool enqueue(T item) override {
        if (mod(_head + 1) == _tail) return false;  // NOTE: this line differs from the paper
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
        if (_tail != _batch_tail) _tail = mod(_tail + 1);  // NOTE: this line differs from the paper
        return out;
    }

    ~b_queue() override { delete[] _buffer; }

  private:
    usize _SIZE;
    usize _BATCH_SIZE;
    usize _BATCH_INCREMENT;
    u32 _WAIT_TIME;
    std::optional<T> *_buffer;

    CACHE_ALIGNED usize _head = 0;
    CACHE_ALIGNED usize _batch_head = 0;

    CACHE_ALIGNED usize _tail = 0;
    CACHE_ALIGNED usize _batch_tail = 0;
    CACHE_ALIGNED usize _batch_history;

    bool _backtrack_deq() {
        if (_batch_history < _BATCH_SIZE) {
            _batch_history = std::min(_BATCH_SIZE, _batch_history + _BATCH_INCREMENT);
        }
        auto batch_size = _batch_history;
        _batch_tail = mod(_tail + batch_size - 1);

        while (!_buffer[_batch_tail]) {
            busy_wait(_WAIT_TIME);
            if (batch_size > 1) {
                batch_size = batch_size >> 1;
                _batch_tail = mod(_tail + batch_size - 1);
            } else
                return false;
        }
        _batch_history = batch_size;
        return true;
    }

    usize mod(const usize index) const { return index % _SIZE; }
};
}  // namespace queues
