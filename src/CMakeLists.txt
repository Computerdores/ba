cmake_minimum_required(VERSION 3.30.5)
project(queue_benchmarks)

add_executable(basic basic.cpp utils.h)

add_executable(test_tsc_freq test_tsc_freq.cpp)
add_executable(test_mc_sync test_mc_sync.cpp)