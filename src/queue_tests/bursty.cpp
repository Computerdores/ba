#include "waiter/bursty.h"

#include <print>

#include "measurer/fine_grained.h"
#include "queues/b_queue.h"
#include "queues/equeue.h"
#include "queues/ff_queue.h"
#include "queues/mc_ring_buffer.h"
#include "runner.h"
#include "waiter/constant_rate.h"

struct {
    usize msg_count = 1'000'000;
    struct rx {
        usize rate = 1'000'000;
    } rx;
    struct tx {
        usize rate = 1'000'000;
        usize burst_size = 2048;
    } tx;
} params;

template <typename Q>
void run_test(Q *queue) {
    Runner<std::remove_pointer_t<Q>, SimplePair<measurer::FineGrained>,
           RXTXPair<waiter::ConstantWait, waiter::Bursty<true>>>
        r(queue, params);
    r.run();
}

int main(const int argc, char *argv[]) {
    if (argc < 2) {
        std::println("missing queue arg");
        return EXIT_FAILURE;
    }

    const auto q_param = std::string(argv[1]);
    if (q_param == "bq") {
        std::println(std::cerr, "using bq");
        queues::b_queue bq(16384, 8192, 64, 50);
        run_test(&bq);
    } else if (q_param == "eq") {
        std::println(std::cerr, "using eq");
        queues::equeue eq(4096, 256, 16384, 50);
        run_test(&eq);
    } else if (q_param == "ffq") {
        std::println(std::cerr, "using ffq");
        queues::ff_queue ffq(1024, 16);
        run_test(&ffq);
    } else if (q_param == "mcrb") {
        std::println(std::cerr, "using mcrb");
        queues::mc_ring_buffer mcrb(16384, 5000);
        run_test(&mcrb);
    } else {
        std::println("invalid queue arg");
        return EXIT_FAILURE;
    }
}
