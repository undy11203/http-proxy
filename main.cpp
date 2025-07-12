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

#include "utils/ConsoleArgs.h"


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


    return 0;
}
