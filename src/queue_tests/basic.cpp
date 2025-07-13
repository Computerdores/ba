
#include "measurer/fine_grained.h"
#include "queues/b_queue.h"
#include "queues/equeue.h"
#include "queues/ff_queue.h"
#include "queues/mc_ring_buffer.h"
#include "runner.h"
#include "waiter/continuous.h"

int main() {
    struct {
        usize msg_count = 1'000'000;
        usize consumer_rate = 1'000'000;
        usize producer_rate = 1'000'000;
    } params;

    queues::b_queue bq(16384, 8192, 64, 50);
    queues::equeue eq(4096, 16384, 50);
    queues::ff_queue ffq(1024, 16);
    queues::mc_ring_buffer mcrb(16384, 5000);

    auto q = &mcrb;
    Runner<std::remove_pointer_t<decltype(q)>, measurer::FineGrained, waiter::Continuous> r(q, params);
    r.run();
}
