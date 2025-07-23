#pragma once

#include <iostream>

#include "measurer/abstract.h"
#include "utils.h"

namespace measurer {

class FineGrained final : public Abstract {
  public:
    explicit FineGrained(const usize msg_count)
        : _msg_count(msg_count), _start(new u64[msg_count]), _end(new u64[msg_count]) {}

    ~FineGrained() override {
        delete _start;
        delete _end;
    }

    inline std::string format_header(std::string prefix) override {
        return std::format("{}_Start,{}_End", prefix, prefix);
    }

    inline std::string row_to_string(const usize index) override {
        return std::format("{},{}", _start[index], _end[index]);
    }

    inline void pre() override { _pre_val = get_timestamp(); }
    inline void post() override {
        const auto post_val = get_timestamp();
        _start[_index] = _pre_val;
        _end[_index] = post_val;
        ++_index;
    }

  private:
    usize _msg_count;
    u64* _start;
    u64* _end;

    CACHE_ALIGNED
    usize _index = 0;
    usize _pre_val = 0;
};

}  // namespace measurer
