cmake_minimum_required(VERSION 3.30.5)
project(queue_benchmarks)

set(LIB_SRC_FILES utils.h queues/ff_queue.h queues/equeue.h)

add_executable(basic basic.cpp ${LIB_SRC_FILES})

add_executable(test_tsc_freq test_tsc_freq.cpp ${LIB_SRC_FILES})
add_executable(test_mc_sync test_mc_sync.cpp ${LIB_SRC_FILES})

set(CMAKE_CXX_FLAGS "-Wno-interference-size")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
