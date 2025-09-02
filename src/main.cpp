#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <print>

#define TOML_HEADER_ONLY 0
#define TOML_IMPLEMENTATION
#define TOML_ENABLE_FORMATTERS 0
#define TOML_ENABLE_FLOAT16 0
#include <toml++/toml.hpp>

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
    // Command Line Arguments
    cxxopts::Options options("benchmarks", "Benchmark different queues.");
    options.set_width(120);

    benchmark bm;
    queue_type qt;
    std::string output_filename;
    bool jitter_value = true;
    bool measure_failed_value = true;
    std::string config_filename;

    options.add_options()                                                                                                       //
        ("h,help", "Print this help text.")                                                                                     //
        ("b,benchmark", "Which benchmark to use (basic|bursty)", cxxopts::value(bm))                                            //
        ("q,queue", "Which queue to benchmark (bq|eq|fflwq|mcrb|ffwdq|lprt)", cxxopts::value(qt))                               //
        ("o,output", "Where to store the result data.", cxxopts::value(output_filename)->default_value("-"))                    //
        ("c,config", "config file for benchmark/queue params", cxxopts::value(config_filename)->default_value("default.toml"))  //
        ("j,jitter", "Whether to enable jitter for the transmit side.", cxxopts::value(jitter_value)->default_value("false"))   //
        ("mf,measure-failed", "Whether to include failed operations in the time measurements",
         cxxopts::value(measure_failed_value)->default_value("false"));

    auto result = options.parse(argc, argv);
    REQUIRE_ARG("benchmark");
    REQUIRE_ARG("queue");

    std::ostream* out = &std::cout;
    std::ofstream ofs;
    if (output_filename != "-" && !output_filename.empty()) {
        ofs = std::ofstream(output_filename);
        if (!ofs.is_open()) {
            std::cerr << "Failed to open file: " << output_filename << std::endl;
            return EXIT_FAILURE;
        }
        out = &ofs;
    }

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    // Config for Queue parameters
    toml::table config;
    try {
        config = toml::parse_file(config_filename);
    } catch (const toml::parse_error& err) {
        std::cerr << "Parsing failed:\n" << err << "\n";
        return EXIT_FAILURE;
    }

    struct {
        usize msg_count;
        struct {
            usize rate;
        } rx;
        struct {
            usize rate;
            usize burst_size;
        } tx;
    } bench_params;
    bench_params.msg_count = config["msg_count"].value_or(1'000'000);
    bench_params.rx.rate = config["rx_rate"].value_or(1'000'000);
    bench_params.tx.rate = config["tx_rate"].value_or(1'000'000);
    bench_params.tx.burst_size = config["tx_burst_size"].value_or(2048);

    // Run specified benchmark
    using QueueVariant = std::variant<std::monostate, queues::b_queue<>, queues::equeue<>, queues::ff_queue<>,
                                      queues::mc_ring_buffer<>, queues::fast_forward<>, queues::lamport<>>;

    QueueVariant queue;
    switch (qt) {
        case queue_type::BQueue: {
            auto params = config["BQueue"];
            auto size = *params["size"].value<usize>(), batch_size = *params["batch_size"].value<usize>(),
                 batch_increment = *params["batch_increment"].value<usize>();
            auto wait_time = *params["wait_time"].value<u64>();
            std::println("Testing BQueue. (size = {}, batch_size = {}, batch_increment = {}, wait_time = {})", size, batch_size,
                         batch_increment, wait_time);
            queue.emplace<queues::b_queue<>>(size, batch_size, batch_increment, wait_time);
            break;
        }
        case queue_type::EQueue: {
            auto params = config["EQueue"];
            auto initial_size = *params["initial_size"].value<u64>(), min_size = *params["min_size"].value<u64>(),
                 max_size = *params["max_size"].value<u64>();
            auto wait_time = *params["wait_time"].value<u32>();
            std::println("Testing E-Queue. (initial_size = {}, min_size = {}, max_size = {}, wait_time = {})", initial_size,
                         min_size, max_size, wait_time);
            queue.emplace<queues::equeue<>>(initial_size, min_size, max_size, wait_time);
            break;
        }
        case queue_type::FastFlowQueue: {
            auto params = config["FastFlow"];
            auto bucket_size = *params["bucket_size"].value<size_t>(),
                 max_bucket_count = *params["max_bucket_count"].value<size_t>();
            std::println("Testing FastFlow Queue. (bucket_size = {}, max_bucket_count = {})", bucket_size, max_bucket_count);
            queue.emplace<queues::ff_queue<>>(bucket_size, max_bucket_count);
            break;
        }
        case queue_type::MCRingBuffer: {
            auto params = config["MCRingBuffer"];
            auto size = *params["size"].value<usize>(), batch_size = *params["batch_size"].value<usize>();
            std::println("Testing MCRingBuffer. (size = {}, batch_size = {})", size, batch_size);
            queue.emplace<queues::mc_ring_buffer<>>(size, batch_size);
            break;
        }
        case queue_type::FastForwardQueue: {
            auto params = config["FastForward"];
            auto size = *params["size"].value<usize>(), adjust_slip_interval = *params["adjust_slip_interval"].value<usize>();
            std::println("Testing FastForward Queue. (size = {}, adjust_slip_interval = {})", size, adjust_slip_interval);
            queue.emplace<queues::fast_forward<>>(size, NS_PER_S / bench_params.tx.rate, adjust_slip_interval);
            break;
        }
        case queue_type::LamportQueue: {
            auto params = config["Lamport"];
            auto size = *params["size"].value<usize>();
            std::println("Testing Lamport Queue. (size = {})", size);
            queue.emplace<queues::lamport<>>(size);
            break;
        }
        default:
            std::cerr << "Unknown queue type!" << std::endl;
            return EXIT_FAILURE;
    }

    using BoolVariant = std::variant<std::true_type, std::false_type>;
    BoolVariant jitter = jitter_value ? BoolVariant {std::true_type {}} : BoolVariant {std::false_type {}};
    BoolVariant measure_failed = measure_failed_value ? BoolVariant {std::true_type {}} : BoolVariant {std::false_type {}};

    return std::visit(
        [&]<typename Q, typename J, typename MF>(Q& q, J, MF) -> int {
            if constexpr (std::is_same_v<Q, std::monostate>)
                return EXIT_FAILURE;
            else {
                std::cout << "Running with jitter: " << J::value << ", measure-failed: " << MF::value << std::endl;
                if (bm == benchmark::basic) {
                    Runner<Q, SimplePair<measurer::FineGrained>, RXTXPair<waiter::ConstantWait, waiter::ConstantRate<J::value>>,
                           MF::value>
                        r(&q, bench_params);
                    r.run();
                    r.write_results(out);
                } else {
                    Runner<Q, SimplePair<measurer::FineGrained>, RXTXPair<waiter::ConstantWait, waiter::Bursty<J::value>>,
                           MF::value>
                        r(&q, bench_params);
                    r.run();
                    r.write_results(out);
                }
                return EXIT_SUCCESS;
            }
        },
        queue, jitter, measure_failed);
}
