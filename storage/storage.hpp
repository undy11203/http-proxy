#pragma once

#include <mutex>
#include <unordered_map>
#include <string>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>


#include <unordered_map>
#include <chrono>
#include <mutex>

class CacheEntry {
public:
    std::string response;
    std::chrono::steady_clock::time_point timestamp;

    CacheEntry() : response(""), timestamp(std::chrono::steady_clock::now()) {}
    CacheEntry(const std::string& response) : response(response), timestamp(std::chrono::steady_clock::now()) {}
};

class Storage {
private:
    std::unordered_map<std::string, CacheEntry> cache;
    size_t current_size;
    size_t max_size;
    std::chrono::seconds ttl;
    std::shared_mutex mutex;

public:
    Storage(size_t initial_size, size_t max_size, std::chrono::seconds ttl)
        : current_size(initial_size), max_size(max_size), ttl(ttl) {}

    std::string get_response(const std::string& uri) {
        std::unique_lock<std::shared_mutex> lock(mutex);
        auto it = cache.find(uri);
        if (it != cache.end()) {
            // Check if the entry is still valid
            if (std::chrono::steady_clock::now() - it->second.timestamp < ttl) {
                return it->second.response;
            } else {
                // Remove expired entry
                current_size -= it->second.response.size();
                cache.erase(it);
            }
        }
        return "";
    }

    void save_response(const std::string& uri, const std::string& response) {
        std::unique_lock<std::shared_mutex> lock(mutex);
        size_t response_size = response.size();

        // Check if adding this response exceeds the cache size
        if (current_size + response_size > max_size) {
            // Log error and return without saving
            std::cerr << "Cache size exceeded. Unable to save response for URI: " << uri << std::endl;
            return;
        }

        // Save the response
        cache[uri] = CacheEntry(response);
        current_size += response_size;
    }
};