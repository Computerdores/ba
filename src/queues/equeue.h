#pragma once

// implements https://doi.org/10.1109/ACCESS.2020.2997071
// parts taken from https://github.com/junchangwang/EQueue/ under GPL-3.0
// didn't use above implementation verbatim because it doesn't seem to be correct (traffic_full and traffic_empty are
// never changed, even though that is required according to the paper)

#include <atomic>
#include <memory>
#include <optional>

#include "../utils.h"

namespace queues {

class equeue {
  public:
    equeue() = delete;

    explicit equeue(const u64 size, const u64 max_size = 1024 * 128, const u64 enlarge_threshold = 1024,
                    const u64 shrink_threshold = 1) {
        _info.head = 0;
        _tail = 0;
        _info.size = size;
        // TODO: test whether cache line alignment makes a difference here
        _data = static_cast<std::uint64_t *>(calloc(max_size, sizeof(u64)));

        _MAX_SIZE = max_size;
        _ENLARGE_THRESHOLD = enlarge_threshold;
        _SHRINK_THRESHOLD = shrink_threshold;
    }

    bool enqueue(const u64 value) {
        assert(value != 0);
        if (_data[_info.head] != 0) {
            _traffic_full++;
            return false;
        }
        const auto temp = _info.head;
        _info.head++;
        if (_info.head >= _info.size) {
            if (_traffic_full - _traffic_empty >= _ENLARGE_THRESHOLD && _info.size * 2 <= _MAX_SIZE) {
                _info.size *= 2;
                _traffic_full = _traffic_empty = 0;
            } else {
                _info.head = 0;
            }
        }
        _data[temp] = value;
        return true;
    }

    std::optional<u64> dequeue() {
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
    static_assert(sizeof(cas_info) == 8);  // make sure cas_info is same size as u64

    union {
        cas_info _info;
        u64 _info_data;
    };

    alignas(CACHE_LINE_SIZE) u32 _tail;
    alignas(CACHE_LINE_SIZE) u64 _traffic_full = 0;
    alignas(CACHE_LINE_SIZE) u64 _traffic_empty = 0;
    alignas(CACHE_LINE_SIZE) u64 *_data;

    // config values
    alignas(CACHE_LINE_SIZE) u64 _MAX_SIZE;
    u64 _ENLARGE_THRESHOLD;
    u64 _SHRINK_THRESHOLD;
};

}  // namespace queues
