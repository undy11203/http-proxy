#include "socket.hpp"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>

socket::socket(int domain, int type, int protocol) {
    fd = ::socket(domain, type, protocol);
    if (fd < 0) {
        perror("socket");
        throw std::runtime_error("Failed to create socket");
    }
}

socket::socket(int client_fd) : fd(client_fd) {
    if (fd < 0) {
        throw std::runtime_error("Invalid file descriptor provided");
    }
}

socket::~socket() {
    // std::cout << "close socket" << std::endl;
    if (fd >= 0) {
        ::close(fd);
    }
}

void socket::close(){
    if(fd >= 0) {
        ::close(fd);
    }
}

void socket::bind(const socketaddress& addr) {
    if (::bind(fd, addr.get_addr(), addr.get_addr_len()) < 0) {
        perror("bind");
        throw std::runtime_error("Failed to bind socket");
    }
}

void socket::listen(int backlog) {
    if (::listen(fd, backlog) < 0) {
        perror("listen");
        throw std::runtime_error("Failed to listen on socket");
    }
}

int socket::accept(socketaddress& addr) {
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    int client_fd;
    client_fd = ::accept(fd, const_cast<struct sockaddr*>(addr.get_addr()), &addrlen);
    return client_fd;
}

ssize_t socket::read(void* buf, size_t count) {
    return ::read(fd, buf, count);
}

ssize_t socket::write(const void* buf, size_t count) {
    return ::write(fd, buf, count);
}

void socket::connect(const socketaddress& addr) {
    if (::connect(fd, addr.get_addr(), addr.get_addr_len()) < 0) {
        perror("connect");
        throw std::runtime_error("Failed to connect socket");
    }
}

int socket::get_fd() const { return fd; }

void socket::set_non_blocking() {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        throw std::runtime_error("Failed to get socket flags");
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL");
        throw std::runtime_error("Failed to set socket to non-blocking mode");
    }
}

void socket::set_reuse_addr() {
    int optval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        throw std::runtime_error("Failed to set SO_REUSEADDR option");
    }
}