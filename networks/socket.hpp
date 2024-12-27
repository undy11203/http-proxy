#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "socketaddress.hpp"

class socket {
    int fd;
public:
    socket() {};
    socket(int domain, int type, int protocol = 0);
    socket(int client_fd);
    ~socket();
    void close();
    void bind(const socketaddress& addr);
    void listen(int backlog);
    int accept(socketaddress& addr);
    ssize_t read(void* buf, size_t count);
    ssize_t write(const void* buf, size_t count);
    int get_fd() const;
    void connect(const socketaddress& addr);
    void set_reuse_addr();

    void set_non_blocking();
};