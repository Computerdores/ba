cmake_minimum_required(VERSION 3.27.0)
project(queue_benchmarks)

set(LIB_SRC_FILES utils.h queues/ff_queue.h queues/equeue.h)

add_executable(basic queue_tests/basic.cpp ${LIB_SRC_FILES})

add_executable(test_tsc_freq hw_tests/test_tsc_freq.cpp ${LIB_SRC_FILES})
add_executable(test_mc_sync hw_tests/test_mc_sync.cpp ${LIB_SRC_FILES})

set(CMAKE_CXX_FLAGS "-Wno-interference-size")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
