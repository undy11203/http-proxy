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
    std::unordered_map<std::string, std::vector<int>> subscribers; // Подписчики по URI
    mutable std::shared_mutex mutex; // Мьютекс для защиты доступа к подписчикам

public:
    // Проверить, есть ли подписка на URI
    bool is_subscribed(const std::string& uri) {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return subscribers.find(uri) != subscribers.end(); // Проверяем наличие подписчиков
    }

    // Подписаться на URI
    void subscribe(const std::string& uri, int client_fd) {
        std::unique_lock<std::shared_mutex> lock(mutex);
        subscribers[uri].push_back(client_fd); // Добавляем клиента в список подписчиков
        // std::cout << "Client " << client_fd << " subscribed to " << uri << std::endl; // Логируем подписку
    }

    // Уведомить подписчиков о новом ответе
    void notify_subscribers(const std::string& uri, const std::string& response) {
        std::shared_lock<std::shared_mutex> lock(mutex);
        auto it = subscribers.find(uri);
        if (it != subscribers.end()) {
            for (int client_fd : it->second) {
                // Отправляем ответ каждому подписчику
                // std::cout << client_fd << " : " << response.size() << std::endl;
                send(client_fd, response.c_str(), response.size(), 0);
                close(client_fd);
            }
        }
        subscribers.erase(it);
    }

    
};