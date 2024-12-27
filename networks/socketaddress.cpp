
#include "socketaddress.hpp"

#include <iostream>

socketaddress::socketaddress() : addrlen(sizeof(struct sockaddr_storage)) {
    std::memset(&addr, 0, sizeof(addr));
}

socketaddress::socketaddress(const char* host, uint16_t port) : socketaddress() {
    struct addrinfo hints, *res, *p;
    int status;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    //slow
    if ((status = getaddrinfo(host, std::to_string(port).c_str(), &hints, &res)) != 0) {
        throw std::runtime_error("getaddrinfo: " + std::string(gai_strerror(status)));
    }

    for (p = res; p != nullptr; p = p->ai_next) {
        if (p->ai_family == AF_INET) { // IPv4
            std::memcpy(&addr, p->ai_addr, p->ai_addrlen);
            addrlen = p->ai_addrlen;
            break;
        } else if (p->ai_family == AF_INET6) { // IPv6
            std::memcpy(&addr, p->ai_addr, p->ai_addrlen);
            addrlen = p->ai_addrlen;
            break;
        }
    }

    freeaddrinfo(res);

    if (addrlen == 0) {
        throw std::runtime_error("No valid address found for host: " + std::string(host));
    }
}

socketaddress::socketaddress(const struct sockaddr* addr, socklen_t addrlen) 
    : addrlen(addrlen) {
    std::memcpy(&this->addr, addr, addrlen);
}

const struct sockaddr* socketaddress::get_addr() const {
    return reinterpret_cast<const struct sockaddr*>(&addr);
}

socklen_t socketaddress::get_addr_len() const {
    return addrlen;
}

void socketaddress::set_port(uint16_t port) {
    if (addr.ss_family == AF_INET) {
        reinterpret_cast<struct sockaddr_in*>(&addr)->sin_port = htons(port);
    } else if (addr.ss_family == AF_INET6) {
        reinterpret_cast<struct sockaddr_in6*>(&addr)->sin6_port = htons(port);
    }
}

uint16_t socketaddress::get_port() const {
    if (addr.ss_family == AF_INET) {
        return ntohs(reinterpret_cast<const struct sockaddr_in*>(&addr)->sin_port);
    } else if (addr.ss_family == AF_INET6) {
        return ntohs(reinterpret_cast<const struct sockaddr_in6*>(&addr)->sin6_port);
    }
    return 0;
}

std::string socketaddress::get_IP() const {
    char ip[INET6_ADDRSTRLEN];
    if (addr.ss_family == AF_INET) {
        inet_ntop(AF_INET, &(reinterpret_cast<const struct sockaddr_in*>(&addr)->sin_addr), ip, INET_ADDRSTRLEN);
    } else if (addr.ss_family == AF_INET6) {
        inet_ntop(AF_INET6, &(reinterpret_cast<const struct sockaddr_in6*>(&addr)->sin6_addr), ip, INET6_ADDRSTRLEN);
    } else {
        return "";
    }
    return std::string(ip);
}