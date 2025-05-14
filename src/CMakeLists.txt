cmake_minimum_required(VERSION 3.30.5)
project(queue_benchmarks)

add_executable(basic basic.cpp utils.h)

add_executable(test_tsc_freq test_tsc_freq.cpp)
add_executable(test_mc_sync test_mc_sync.cpp)

set(CMAKE_CXX_FLAGS "-Wno-interference-size")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
