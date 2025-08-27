#include <cxxopts.hpp>
#include <iostream>

#include "measurer/fine_grained.h"
#include "queues/b_queue.h"
#include "queues/equeue.h"
#include "queues/fast_forward.h"
#include "queues/ff_queue.h"
#include "queues/lamport.h"
#include "queues/mc_ring_buffer.h"
#include "runner.h"
#include "waiter/bursty.h"
#include "waiter/constant_rate.h"
#include "waiter/constant_wait.h"

#define REQUIRE_ARG(name)                                              \
    if (!result.count(name)) {                                         \
        std::cerr << "Error: Required option '" name "' not present."; \
        return EXIT_FAILURE;                                           \
    }

enum class queue_type {
    BQueue,
    EQueue,
    FastFlowQueue,
    MCRingBuffer,
    FastForwardQueue,
    LamportQueue,
};

std::istream& operator>>(std::istream& in, queue_type& qt) {
    std::string token;
    in >> token;

    std::unordered_map<std::string, queue_type> lookup = {{"bq", queue_type::BQueue},
                                                          {"eq", queue_type::EQueue},
                                                          {"fflwq", queue_type::FastFlowQueue},
                                                          {"mcrb", queue_type::MCRingBuffer},
                                                          {"ffwdq", queue_type::FastForwardQueue},
                                                          {"lprt", queue_type::LamportQueue}};

    const auto it = lookup.find(token);
    if (it == lookup.end()) {
        throw cxxopts::exceptions::invalid_option_format("Invalid queue type: " + token);
    }
    qt = it->second;
    return in;
}

enum class benchmark {
    basic,
    bursty,
};

std::istream& operator>>(std::istream& in, benchmark& bm) {
    std::string token;
    in >> token;

    std::unordered_map<std::string, benchmark> lookup = {{"basic", benchmark::basic}, {"bursty", benchmark::bursty}};

    const auto it = lookup.find(token);
    if (it == lookup.end()) {
        throw cxxopts::exceptions::invalid_option_format("Invalid benchmark: " + token);
    }
    bm = it->second;
    return in;
}

int main(const int argc, char** argv) {
    cxxopts::Options options("benchmarks", "Benchmark different queues.");
    options.set_width(120);

    benchmark bm;
    queue_type qt;

    options.add_options()("b,benchmark", "Which benchmark to use (basic|bursty)", cxxopts::value(bm))(
        "q,queue", "Which queue to benchmark (bq|eq|fflwq|mcrb|ffwdq|lprt)", cxxopts::value(qt))(
        "h,help", "Print this help text.");

    auto result = options.parse(argc, argv);
    REQUIRE_ARG("benchmark");
    REQUIRE_ARG("queue");

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    using QueueVariant = std::variant<std::monostate, queues::b_queue<>, queues::equeue, queues::ff_queue<>,
                                      queues::mc_ring_buffer<>, queues::fast_forward<>, queues::lamport<>>;

    QueueVariant queue;
    switch (qt) {
        case queue_type::BQueue:
            queue.emplace<queues::b_queue<>>(16384, 8192, 64, 50);
            break;
        case queue_type::EQueue:
            queue.emplace<queues::equeue>(4096, 256, 16384, 50);
            break;
        case queue_type::FastFlowQueue:
            queue.emplace<queues::ff_queue<>>(1024, 16);
            break;
        case queue_type::MCRingBuffer:
            queue.emplace<queues::mc_ring_buffer<>>(16384, 5000);
            break;
        case queue_type::FastForwardQueue:
            queue.emplace<queues::fast_forward<>>(16384, NS_PER_S / params.tx.rate, params.msg_count);
            break;
        case queue_type::LamportQueue:
            queue.emplace<queues::lamport<>>(16384);
            break;
        default:
            std::cerr << "Unknown queue type!" << std::endl;
            return EXIT_FAILURE;
    }

    return std::visit(
        [&]<typename Q>(Q& q) -> int {
            if constexpr (std::is_same_v<Q, std::monostate>)
                return EXIT_FAILURE;
            else {
                struct {
                    usize msg_count = 1'000'000;
                    struct {
                        usize rate = 1'000'000;
                    } rx;
                    struct {
                        usize rate = 1'000'000;
                        usize burst_size = 2048;
                    } tx;
                } params;
                if (bm == benchmark::basic) {
                    Runner<Q, SimplePair<measurer::FineGrained>,
                           RXTXPair<waiter::ConstantWait, waiter::ConstantRate<true>>>
                        r(&q, params);
                    r.run();
                } else {
                    Runner<Q, SimplePair<measurer::FineGrained>, RXTXPair<waiter::ConstantWait, waiter::Bursty<true>>>
                        r(&q, params);
                    r.run();
                }
                return EXIT_SUCCESS;
            }
        },
        queue);
}
