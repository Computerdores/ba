#include "ff_queue.hpp"

auto channel = ff_queue(1024, 10);

int main() {
    auto islot = static_cast<int *>(channel.enqueue_prepare(sizeof(int)));
    *islot = 42;
    channel.enqueue_commit();
    auto oslot = static_cast<int *>(channel.dequeue_prepare());
    assert(*oslot == 42);
    channel.dequeue_commit();
    return 0;
}