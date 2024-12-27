#pragma once
 

#include <unordered_map>
#include <vector>
#include <string>
#include <shared_mutex>
#include <algorithm>
#include <iostream>
#include <mutex>
#include <sys/socket.h>
#include <unistd.h>

class SubscribeManager {
private:
    std::unordered_map<std::string, std::vector<int>> subscribers; // Subscribers by URI
    mutable std::shared_mutex mutex;

public:
    // Check if there is a subscription to the URI
    bool is_subscribed(const std::string& uri) {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return subscribers.find(uri) != subscribers.end();
    }

    // Subscribe to URI
    void subscribe(const std::string& uri, int client_fd) {
        std::unique_lock<std::shared_mutex> lock(mutex);
        subscribers[uri].push_back(client_fd); // Adding a client to subscribers
        // std::cout << "Client " << client_fd << " subscribed to " << uri << std::endl; // Логируем подписку
    }

    // Notify subscribers about ready reply
    void notify_subscribers(const std::string& uri, const std::string& response) {
        std::shared_lock<std::shared_mutex> lock(mutex);
        auto it = subscribers.find(uri);
        if (it != subscribers.end()) {
            for (int client_fd : it->second) {
                // std::cout << client_fd << " : " << response.size() << std::endl;
                send(client_fd, response.c_str(), response.size(), 0);
                close(client_fd);
            }
        }
        subscribers.erase(it); // delete subscribe
    }

    
};