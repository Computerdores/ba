cmake_minimum_required(VERSION 3.27.0)
set(CMAKE_CXX_STANDARD 23)
project(queue_benchmarks)

include(FetchContent)
FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(benchmarks main.cpp)

target_link_libraries(benchmarks PRIVATE tomlplusplus::tomlplusplus)

add_executable(test_clock_gettime hw_tests/test_clock_gettime.cpp)
add_executable(test_constant_tsc hw_tests/test_constant_tsc.cpp)
add_executable(test_timestamp_methods hw_tests/test_timestamp_methods.cpp)

set(CMAKE_CXX_FLAGS "-Wno-interference-size")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
