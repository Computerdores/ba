#pragma once
#include "abstract.h"
#include "utils.h"

namespace queues {

template <typename T = u64>
class fast_forward final : public Queue<T> {
  public:
    explicit fast_forward(const usize size, const usize avg_enqueue_interval, const usize adjust_slip_interval)
        : _SIZE(size), _AVG_ENQUEUE_INTERVAL(avg_enqueue_interval), _ADJUST_SLIP_INTERVAL(adjust_slip_interval) {
        _buffer = static_cast<T *>(calloc(size, sizeof(T)));
    }

    bool enqueue(T item) override {
        if (_buffer[_head] != NULL) {
            return false;
        }
        _buffer[_head] = item;
        _head = _next(_head);
        return true;
    }

    std::optional<T> dequeue() override {
        if (_tail % _ADJUST_SLIP_INTERVAL == 0) {
            _adjust_slip();
        }
        auto data = _buffer[_tail];
        if (data == NULL) {
            _adjust_slip();
            return std::nullopt;
        }
        _buffer[_tail] = NULL;
        _tail = _next(_tail);
        return data;
    }

  private:
    CACHE_ALIGNED T *_buffer;
    CACHE_ALIGNED usize _head = 0;
    CACHE_ALIGNED usize _tail = 0;

    CACHE_ALIGNED
    usize _SIZE;
    usize _AVG_ENQUEUE_INTERVAL;
    usize _ADJUST_SLIP_INTERVAL;
    const usize DANGER = 2 * CACHE_LINE_SIZE;
    const usize GOOD = 6 * CACHE_LINE_SIZE;

    void _adjust_slip() const {
        const usize dist = _distance();
        if (dist < DANGER) {
            auto dist_old = 0;
            do {
                dist_old = dist;
                busy_wait(_AVG_ENQUEUE_INTERVAL * ((GOOD + 1) - dist));
                dist = _distance();
            } while (dist < GOOD && dist > dist_old);
        }
    }

    usize _distance() const {
        return _head > _tail ? _head - _tail : _tail - _head;  // NOTE: paper does not specify how to implement this
    }
    usize _next(const usize index) const { return (index + 1) % _SIZE; }
};

}  // namespace queues
