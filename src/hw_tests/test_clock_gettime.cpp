#include <ctime>
#include <iostream>

int main() {
    timespec ts0 = {};
    timespec ts1 = {};
    for (int i = 0; i < 1000; i++) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts0);
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts1);

        if (ts0.tv_sec != ts1.tv_sec) continue;

        std::cout << ts1.tv_nsec - ts0.tv_nsec << std::endl;
    }
}
