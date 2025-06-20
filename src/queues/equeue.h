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
 * - implements equeue without batching
 * - divergences from paper:
 *   - enqueue ensures that _info.size doesn't exceed the buffer size
 *   - one if statement in _enqueue_detect_batching_size differs because the paper referenced an undefined variable
 *   - enqueue method increments traffic_full when batching even-though paper pseudocode doesn't because dynamic
 *     resizing would otherwise not work
 */
class equeue final : public Queue<u64> {
    // TODO: switch to optionals?
  public:
    equeue() = delete;

    explicit equeue(const u64 size, const u64 max_size, const u32 wait_time, const u64 enlarge_threshold = 1024,
                    const u64 shrink_threshold = 1, const u64 max_batch_size = -1, const u64 min_batch_size = -1)
        : _MAX_SIZE(max_size),
          _ENLARGE_THRESHOLD(enlarge_threshold),
          _SHRINK_THRESHOLD(shrink_threshold),
          _WAIT_TIME(wait_time) {
        _info.head = 0;
        _info.size = size;
        // TODO: test whether cache line alignment makes a difference here
        _data = static_cast<std::uint64_t *>(calloc(_MAX_SIZE, sizeof(u64)));

        _MAX_BATCH_SIZE = max_batch_size != -1 ? max_batch_size : size >> 2;
        _MIN_BATCH_SIZE = min_batch_size != -1 ? min_batch_size : 1;
    }

    bool enqueue(const u64 value) override {
        assert(value != 0);
        if (_info.head == _batch_head) {
            if (!_enqueue_detect_batching_size()) {
                ++_traffic_full;  // NOTE: this line differs from the paper
                return false;
            }
        }
        const auto temp = _info.head;
        ++_info.head;
        if (_info.head >= _info.size) {
            if (_traffic_full - _traffic_empty >= _ENLARGE_THRESHOLD &&
                _info.size * 2 <= _MAX_SIZE) {  // NOTE: this line differs from the paper
                _info.size *= 2;
                _traffic_full = _traffic_empty = 0;
            } else {
                _info.head = 0;
            }
        }
        _data[temp] = value;
        return true;
    }

    std::optional<u64> dequeue() override {
        if (_data[_tail] == 0) {
            _traffic_empty++;
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
                if (info_temp.head <= info_temp.size / 2) {
                    info_temp_new.size = info_temp.size / 2;
                    if (__sync_bool_compare_and_swap(&_info_data, info_temp_data, info_temp_new_data)) {
                        _traffic_empty = _traffic_full = 0;
                    } else {
                        goto attempt_cas;
                    }
                }
            }
            _tail = 0;
        }

        auto temp_val = _data[temp_idx];
        _data[temp_idx] = 0;
        return temp_val;
    }

  private:
    struct __attribute__((packed)) cas_info {
        u32 head;
        u32 size;
    };
    static_assert(sizeof(cas_info) == sizeof(u64));

    union {
        cas_info _info;
        u64 _info_data;
    };

    CACHE_ALIGNED u32 _batch_head = 0;
    CACHE_ALIGNED u32 _tail = 0;
    CACHE_ALIGNED u64 _traffic_full = 0;
    CACHE_ALIGNED u64 _traffic_empty = 0;
    CACHE_ALIGNED u64 *_data;

    // config values
    CACHE_ALIGNED
    u64 _MAX_SIZE;
    u64 _ENLARGE_THRESHOLD;
    u64 _SHRINK_THRESHOLD;
    u64 _MAX_BATCH_SIZE;
    u64 _MIN_BATCH_SIZE;
    u32 _WAIT_TIME;

    bool _enqueue_detect_batching_size() {
        auto size = _MAX_BATCH_SIZE;
        auto head = mod(_info.head + size);
        while (_data[head] && size > _MIN_BATCH_SIZE) {
            busy_wait(_WAIT_TIME);
            size = size >> 1;
            head = mod(_info.head + size);
        }
        if (size <= _MIN_BATCH_SIZE)  // NOTE: in the paper this references the undefined variable `batch_size`
            return false;
        _batch_head = head;
        return true;
    }

    u32 mod(const u32 value) const { return value % _info.size; }
};

}  // namespace queues
