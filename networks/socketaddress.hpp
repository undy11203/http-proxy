#pragma once

#include <string>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>

class socketaddress {
public:
    socketaddress();
    socketaddress(const char* ip, uint16_t port);
    socketaddress(const struct sockaddr* addr, socklen_t addrlen);

    const struct sockaddr* get_addr() const;
    socklen_t get_addr_len() const;

    void set_port(uint16_t port);
    uint16_t get_port() const;

    std::string get_IP() const;

private:
    struct sockaddr_storage addr;
    socklen_t addrlen;
};