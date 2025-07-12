#pragma once
#include <iostream>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class ProxySession {
private:
    tcp::socket client_sock_;
    tcp::socket server_sock_;
    enum { buffer_size = 8192 };
    char client_buffer_[buffer_size];
    char server_buffer_[buffer_size];
    std::string target_host_;
    unsigned short target_port_;
    std::size_t client_bytes_transferred_;

    void handle_client_read(const boost::system::error_code& error, std::size_t bytes_transferred);
    void connect_to_server();
    void handle_server_connect(const boost::system::error_code& error);
    void handle_server_write(const boost::system::error_code& error, std::size_t bytes_transferred);
    void read_from_server();
    void handle_server_read(const boost::system::error_code& error, std::size_t bytes_transferred);
    void write_to_client(const boost::system::error_code& error, std::size_t bytes_transferred);
public:
    explicit ProxySession(boost::asio::io_context& io_context);
    tcp::socket& client_socket();

    void start();
};

class ProxyServer {
private:
	tcp::acceptor acceptor_;
    boost::asio::io_context& io_context_;
public:
	ProxyServer(boost::asio::io_context& io_context, short port);
	void handle_accept(ProxySession* new_session, const boost::system::error_code& error);
	void start_accept();

};