#pragma once

#include <iostream>

#include "measurer/abstract.h"
#include "utils.h"

namespace measurer {

class FineGrained final : public Abstract {
  public:
    explicit FineGrained(const usize msg_count)
        : _msg_count(msg_count),
          _tx_start(new u64[msg_count]),
          _tx_end(new u64[msg_count]),
          _rx_start(new u64[msg_count]),
          _rx_end(new u64[msg_count]) {}

    ~FineGrained() override {
        delete _tx_start;
        delete _tx_end;
        delete _rx_start;
        delete _rx_end;
    }

    inline void print_results() override {
        std::cout << "TX_Start,TX_End,RX_Start,RX_End" << std::endl;
        for (usize i = 0; i < _msg_count; i++) {
            std::cout << _tx_start[i] << "," << _tx_end[i] << "," << _rx_start[i] << "," << _rx_end[i] << std::endl;
        }
    }

    inline void pre_rx() override { _rx_data.pre_val = get_timestamp(); }
    inline void post_rx() override {
        const auto post_val = get_timestamp();
        _rx_start[_rx_data.index] = _rx_data.pre_val;
        _rx_end[_rx_data.index] = post_val;
        ++_rx_data.index;
    }

    inline void pre_tx() override { _tx_data.pre_val = get_timestamp(); }
    inline void post_tx() override {
        const auto post_val = get_timestamp();
        _tx_start[_tx_data.index] = _tx_data.pre_val;
        _tx_end[_tx_data.index] = post_val;
        ++_tx_data.index;
    }

  private:
    usize _msg_count;
    u64* _tx_start;
    u64* _tx_end;
    u64* _rx_start;
    u64* _rx_end;

    CACHE_ALIGNED
    struct {
        usize index;
        usize pre_val;
    } _rx_data = {};

    CACHE_ALIGNED
    struct {
        usize index;
        usize pre_val;
    } _tx_data = {};
};

}  // namespace measurer
