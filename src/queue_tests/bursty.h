#pragma once
#include "utils.h"

struct test_parameters {
    usize msg_count = 1'000'000;
    usize consumer_rate = 1'000'000;
    usize producer_rate = 1'000'000;
    usize burst_size = 2048;
};
