// main.cpp
#include <iostream>
#include <cstring>
#include <iterator>
#include <unistd.h>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <getopt.h>
#include <csignal>
#include <fcntl.h>
#include <unordered_map>
#include <algorithm>
#include <future>
#include <mutex>
#include <list>
#include <functional>
#include <shared_mutex>
#include <unordered_map>
#include <iostream>
#include <unordered_map>
#include <list>
#include <functional>
#include <shared_mutex>

#include "networks/socketaddress.hpp"
#include "networks/socket.hpp"
#include "http-parse/httpparse.hpp"
#include "thread_pool/threadpool.hpp"
#include "storage/storage.hpp"
#include "storage/subscribemanager.hpp"

std::atomic<bool> shutdown_requested(false);

// Signal handler
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        shutdown_requested.store(true);
    }
}


class Task {
private:
int client_fd;
Storage& storage;
SubscribeManager& manager;
std::shared_mutex& mutex;

public:
    Task(int client_fd, Storage& storage, SubscribeManager& manager, std::shared_mutex& mutex) : 
            client_fd(client_fd), storage(storage), manager(manager), mutex(mutex) {}

    void execute(){
        std::string request;
        char temp_buffer[1024];
        ssize_t bytes_read;

        while ((bytes_read = read(client_fd, temp_buffer, sizeof(temp_buffer) - 1)) > 0) {
            request.append(temp_buffer, bytes_read);

            if (request.find("\r\n\r\n") != std::string::npos) {
                break;
            }
        }

        if (bytes_read < 0) {
            close(client_fd);
            return;
        }

        httpparser parser(request, true);
        std::string uri = parser.get_uri();

        {
            std::unique_lock<std::shared_mutex> lock(mutex);
            std::string cached_response = storage.get_response(uri);
            if (!cached_response.empty() && cached_response != "") {
                // std::cout << cached_response.size() << std::endl;
                send(client_fd, cached_response.c_str(), cached_response.size(), 0);
                close(client_fd);
                return;
            }

            if (manager.is_subscribed(uri)) {
                manager.subscribe(uri, client_fd);
                return; 
            }
            manager.subscribe(uri, client_fd);
        }

        class socket server_socket(AF_INET, SOCK_STREAM, 0);
        server_socket.connect(socketaddress(parser.get_host().c_str(), 80));

        std::string update_requset = httpparser::update_version(request);
        server_socket.write(update_requset.c_str(), update_requset.size());

        std::string response;
        while ((bytes_read = server_socket.read(temp_buffer, sizeof(temp_buffer))) > 0) {
            response.append(temp_buffer, bytes_read);
            std::string delimiter = "\r\n\r\n";
            size_t pos = response.find(delimiter);
            if (pos != std::string::npos) {
                std::string headers = response.substr(0, pos);
                httpparser response_parser(headers, false);


                int status_code = response_parser.get_status_code();
                if (status_code >= 300 && status_code < 400) {
                    std::string location = response_parser.get_header("Location");
                    if (!location.empty()) {
                        uri = location;

                        parser.set_url(uri);
                        std::string update_request = httpparser::update_version(parser.get_request());

                        server_socket.write(update_request.c_str(), update_request.size());
                        response.clear();
                        continue;
                    }
                }


                std::string content_length_str = response_parser.get_header("Content-Length");
                size_t content_length = content_length_str.empty() ? 0 : std::stoul(content_length_str);

                size_t remaining_data_size = response.size() - (pos + 4);
                if (remaining_data_size >= content_length) {
                    break;
                }
            }
        }

        {
            std::unique_lock<std::shared_mutex> lock(mutex);
            std::string delimiter = "\r\n\r\n";
            size_t pos = response.find(delimiter);
            std::string headers = response.substr(0, pos);
            httpparser response_parser(headers, false);
            int status_code = response_parser.get_status_code();
            if(status_code < 300)
                storage.save_response(uri, response);
            manager.notify_subscribers(uri, response);
        }

        // std::cout << response.size() << std::endl;
    }

};


class CachingProxyServer {
private:
int port;
ThreadPool pool;
Storage storage;
SubscribeManager manager;
std::shared_mutex mutex;

public:
CachingProxyServer(int port, int max_threads, size_t initial_cache_size, size_t max_cache_size, std::chrono::seconds cache_ttl)
        : port(port), pool(max_threads), storage(initial_cache_size, max_cache_size, cache_ttl) {}

void start() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    class socket server_socket(AF_INET, SOCK_STREAM, 0);
    server_socket.set_reuse_addr();
    server_socket.set_non_blocking();
    server_socket.bind(socketaddress("0.0.0.0", port));
    server_socket.listen(100);

    std::cout << "Proxy server started on port " << port << std::endl;

    while (!shutdown_requested.load()) {
        socketaddress client_addr;
        int client_fd = server_socket.accept(client_addr);
        if (client_fd > 0) {
            pool.enqueue([this, client_fd] {
                Task task(client_fd, storage, manager, mutex);
                task.execute();
            });
        }
    }

    std::cout << "Received shutdown signal" << std::endl;

    // Ожидание завершения всех потоков
    pool.close();
    std::cout << "All threads have finished" << std::endl;

    server_socket.close();
    std::cout << "Proxy finished" << std::endl;
}

};



void print_help() {
    std::cout << "Usage: proxy [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --port <port>              Port to listen on (default: 8080)" << std::endl;
    std::cout << "  --max-client-threads <num> Maximum number of client threads (default: 4)" << std::endl;
    std::cout << "  --help                     Display this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    size_t max_threads = 4;
    size_t initial_cache_size = 1 * 1024 * 1024; // 1 MB
    size_t max_cache_size = 10 * 1024 * 1024; // 10 MB
    std::chrono::seconds cache_ttl(5); // 5 seconds

    static struct option long_options[] = {
        {"port", required_argument, 0, 'p'},
        {"max-client-threads", required_argument, 0, 't'},
        {"cache-initial-size", required_argument, 0, 'i'},
        {"cache-max-size", required_argument, 0, 'm'},
        {"cache-ttl", required_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "p:t:i:m:l:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'p':
                port = std::stoi(optarg);
                break;
            case 't':
                max_threads = std::stoul(optarg);
                break;
            case 'i':
                initial_cache_size = std::stoul(optarg);
                break;
            case 'm':
                max_cache_size = std::stoul(optarg);
                break;
            case 'l':
                cache_ttl = std::chrono::seconds(std::stoul(optarg));
                break;
            case 'h':
                print_help();
                return 0;
            default:
                abort();
        }
    }

    try {
        // SimpleProxyServer proxy(port, max_threads);
        CachingProxyServer proxy(port, max_threads, initial_cache_size, max_cache_size, cache_ttl);
        proxy.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}