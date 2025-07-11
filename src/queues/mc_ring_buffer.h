#pragma once
#include "abstract.h"
#include "utils.h"

namespace queues {

template <typename T = u64, bool BLOCK_ON_EMPTY = false, bool BLOCK_ON_FULL = false>
class mc_ring_buffer final : public Queue<T> {
  public:
    mc_ring_buffer(const usize size, const usize batch_size) : _size(size), _batch_size(batch_size) {
        _buffer = new T[_size];
    }

    ~mc_ring_buffer() override { delete[] _buffer; }

    bool enqueue(T item) override {
        const auto after_next_write = _next(_next_write);
        if (after_next_write == _local_read) {
            while (after_next_write == _read) {
                if (!BLOCK_ON_FULL) return false;
            }
            _local_read = _read;
        }
        _buffer[_next_write] = item;
        _next_write = after_next_write;
        _write_batch++;
        if (_write_batch >= _batch_size) {
            _write = _next_write;
            _write_batch = 0;
        }
        return true;
    }

    std::optional<T> dequeue() override {
        if (_next_read == _local_write) {
            while (_next_read == _write) {
                if (!BLOCK_ON_EMPTY) return std::nullopt;
            }
            _local_write = _write;
        }
        T out = _buffer[_next_read];
        _next_read = _next(_next_read);
        _read_batch++;
        if (_read_batch >= _batch_size) {
            _read = _next_read;
            _read_batch = 0;
        }
        return out;
    }

  private:
    // shared control variables
    CACHE_ALIGNED
    volatile usize _read = 0;
    volatile usize _write = 0;

    // consumer's local control variables
    CACHE_ALIGNED
    usize _local_write = 0;
    usize _next_read = 0;
    usize _read_batch = 0;

    // producer's local control variables
    CACHE_ALIGNED
    usize _local_read = 0;
    usize _next_write = 0;
    usize _write_batch = 0;

    // constants
    CACHE_ALIGNED
    usize _size;
    usize _batch_size;

    // buffer
    CACHE_ALIGNED
    T *_buffer;

    usize _next(const usize current) const { return (current + 1) % _size; }
};

}  // namespace queues
