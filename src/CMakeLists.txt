cmake_minimum_required(VERSION 3.27.0)
set(CMAKE_CXX_STANDARD 23)
project(queue_benchmarks)

set(LIB_SRC_FILES utils.h queues/ff_queue.h queues/equeue.h queues/b_queue.h)

include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(basic queue_tests/basic.cpp ${LIB_SRC_FILES})
add_executable(bursty queue_tests/bursty.cpp ${LIB_SRC_FILES})

add_executable(test_tsc_freq hw_tests/test_tsc_freq.cpp ${LIB_SRC_FILES})
add_executable(test_mc_sync hw_tests/test_mc_sync.cpp ${LIB_SRC_FILES})
add_executable(test_clock_gettime hw_tests/test_clock_gettime.cpp ${LIB_SRC_FILES})
add_executable(test_constant_tsc hw_tests/test_constant_tsc.cpp ${LIB_SRC_FILES})

set(CMAKE_CXX_FLAGS "-Wno-interference-size")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
