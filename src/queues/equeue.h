#pragma once

// implements https://doi.org/10.1109/ACCESS.2020.2997071 (repo is now down after I wrote them an email...)
// parts taken from https://github.com/junchangwang/EQueue/ under GPL-3.0
// didn't use above implementation verbatim because it doesn't seem to be entirely match the paper

#include <memory>
#include <optional>

#include "queues/abstract.h"
#include "utils.h"

namespace queues {

/**
 * State of this implementation:
 * - implements equeue with batching
 * - divergences from paper:
 *   - enqueue ensures that _info.size doesn't exceed the buffer size
 *   - _enqueue_detect_batching_size differs because the paper and ref impl differ
 *   - enqueue method increments traffic_full when batching even-though paper pseudocode doesn't because dynamic
 *     resizing would otherwise not work
 */
template <typename T = u64, T EMPTY = 0>
class equeue final : public Queue<T> {
    // TODO: switch to optionals?
  public:
    equeue() = delete;

    // paper: "SHRINK_THRESHOLD is by default set to 128."
    // default values for ENLARGE_THRESHOLD and BATCH_SLICE are not defined in the paper, took them from ref impl
    // instead
    explicit equeue(const u64 size, const u64 min_size, const u64 max_size, const u32 wait_time,
                    const u64 enlarge_threshold = 1024, const u64 shrink_threshold = 128, const u64 max_batch_size = -1,
                    const u64 min_batch_size = 128)
        : _MIN_SIZE(min_size),
          _MAX_SIZE(max_size),
          _ENLARGE_THRESHOLD(enlarge_threshold),
          _SHRINK_THRESHOLD(shrink_threshold),
          _WAIT_TIME(wait_time) {
        _info.head = 0;
        _info.size = size;
        _data = new T[_MAX_SIZE](EMPTY);

        // paper: "[_MAX_BATCH_SIZE] is set by default to one-quarter of the queue size"
        _MAX_BATCH_SIZE = max_batch_size != -1 ? max_batch_size : size >> 2;
        _MIN_BATCH_SIZE = min_batch_size;
    }

    bool enqueue(const T value) override {
        assert(value != 0);
        if (_info.head == _batch_head) {
            if (!_enqueue_detect_batching_size()) {
                _traffic_full = _traffic_full + 1;  // NOTE: this line differs from the paper
                return false;
            }
        }
        const auto temp = _info.head;
        ++_info.head;
        if (_info.head >= _info.size) {
            if (_traffic_full - _traffic_empty >= _ENLARGE_THRESHOLD &&
                _info.size * 2 <= _MAX_SIZE) {  // NOTE: this line differs from the paper
                _info.size *= 2;
                _traffic_full = 0;
                _traffic_empty = 0;
            } else {
                _info.head = 0;
            }
        }
        _data[temp] = value;
        return true;
    }

    std::optional<T> dequeue() override {
        if (_data[_tail] == EMPTY) {
            _traffic_empty = _traffic_empty + 1;
            return std::nullopt;
        }
        const auto temp_idx = _tail;
        _tail++;

    attempt_cas:
        if (_tail >= _info.size) {
            if (_traffic_empty - _traffic_full >= _SHRINK_THRESHOLD) {
                union {
                    cas_info info_temp;
                    u64 info_temp_data;
                };
                union {
                    cas_info info_temp_new;
                    u64 info_temp_new_data;
                };
                info_temp = info_temp_new = _info;
                // Note: The following _MIN_SIZE check differs from the ref impl (and is missing in the paper).
                //       Afaict, the check in the ref impl is wrong since it would allow the size to fall below
                //       the minimum allowed size (checks the previous size not the new one).
                if (info_temp.head <= info_temp.size / 2 && info_temp.size / 2 >= _MIN_SIZE) {
                    info_temp_new.size = info_temp.size / 2;
                    if (__sync_bool_compare_and_swap(&_info_data, info_temp_data, info_temp_new_data)) {
                        _traffic_empty = 0;
                        _traffic_full = 0;
                    } else {
                        goto attempt_cas;
                    }
                }
            }
            _tail = 0;
        }

        auto temp_val = _data[temp_idx];
        _data[temp_idx] = EMPTY;
        return temp_val;
    }

  private:
    struct __attribute__((packed)) cas_info {
        u32 head;
        volatile u32 size;
    };
    static_assert(sizeof(cas_info) == sizeof(u64));

    union {
        CACHE_ALIGNED cas_info _info;
        CACHE_ALIGNED u64 _info_data;
    };

    CACHE_ALIGNED u32 _batch_head = 0;
    CACHE_ALIGNED u32 _tail = 0;
    CACHE_ALIGNED volatile u64 _traffic_full = 0;
    CACHE_ALIGNED volatile u64 _traffic_empty = 0;

    // buffer
    CACHE_ALIGNED
    volatile T *_data;

    // config values
    u64 _MIN_SIZE;
    u64 _MAX_SIZE;
    u64 _ENLARGE_THRESHOLD;
    u64 _SHRINK_THRESHOLD;
    u64 _MAX_BATCH_SIZE;
    u64 _MIN_BATCH_SIZE;
    u32 _WAIT_TIME;

    bool _enqueue_detect_batching_size() {
        // NOTE: This function is structured somewhat differently between ref impl and paper.
        //       This implementation is closer to the ref impl.
        auto size = _MAX_BATCH_SIZE;
        auto head = _mod(_info.head + size);
        while (_data[head] != EMPTY) {
            busy_wait(_WAIT_TIME);
            if (size > _MIN_BATCH_SIZE) {
                size = size >> 1;
                head = _mod(_info.head + size);
            } else {
                return false;
            }
        }
        _batch_head = head;
        return true;
    }

    u32 _mod(const u32 value) const { return value % _info.size; }
};

}  // namespace queues
