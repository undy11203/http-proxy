#include <iostream>
#include <cstring>
#include <iterator>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <csignal>
#include <fcntl.h>
#include <unordered_map>
#include <algorithm>
#include <future>
#include <mutex>
#include <list>
#include <functional>
#include <shared_mutex>
#include <boost/asio.hpp>

#include "utils/ConsoleArgs.h"
#include "proxy_main/Proxy.h"



std::atomic<bool> shutdown_requested(false);

// Signal handler
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        shutdown_requested.store(true);
    }
}

int main(int argc, char* argv[]) {
    // Register signal handler
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    ConsoleArgs args;

    if (!ConsoleArgsParser::run(args, argc, argv)) {
        return 1;
    }

    // Application logic here...
    try {
        boost::asio::io_context io_context;
        ProxyServer server(io_context, static_cast<short>(8080));

        std::cout << "HTTP Proxy is running on port " << argv[1] << "...\n";
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
