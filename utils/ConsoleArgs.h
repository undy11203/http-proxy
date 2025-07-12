#pragma once
#include <chrono>

#include <boost/program_options.hpp>
#include <iostream>

struct ConsoleArgs {
    int port = 8080;
    size_t max_thread = 0;
    size_t initial_cache_size = 0;
    size_t max_cache_size = 0;
    std::chrono::seconds cache_ttl;
};

class ConsoleArgsParser {
public:
    static bool run(ConsoleArgs& args, int argc, char* argv[]);
};

bool ConsoleArgsParser::run(ConsoleArgs& args, int argc, char* argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Display this help message")
        ("port,p", po::value<int>(&args.port)->default_value(8080), "Port to listen on")
        ("max-client-threads,t", po::value<size_t>(&args.max_thread)->default_value(0), "Maximum number of client threads")
        ("cache-initial-size,i", po::value<size_t>(&args.initial_cache_size)->default_value(0), "Initial cache size")
        ("cache-max-size,m", po::value<size_t>(&args.max_cache_size)->default_value(0), "Max cache size")
        ("cache-ttl,l", po::value<size_t>()->default_value(0), "Cache write lifetime in seconds")
    ;

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        std::cout << desc << std::endl;
        return false;
    }

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return true;
    }

    if (vm.count("cache-ttl")) {
        args.cache_ttl = std::chrono::seconds(vm["cache-ttl"].as<size_t>());
    }

    return true;

}