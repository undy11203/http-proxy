#include "Proxy.h"

// Session Impl

ProxySession::ProxySession(boost::asio::io_context& io_context)
    : client_sock_(io_context), server_sock_(io_context) {}

tcp::socket& ProxySession::client_socket() {
    return client_sock_;
}

void ProxySession::start() {
    client_sock_.async_read_some(boost::asio::buffer(client_buffer_, buffer_size),
        [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
            handle_client_read(error, bytes_transferred);
        });
}

void ProxySession::handle_client_read(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (error) return;

    std::string request(client_buffer_, bytes_transferred);
    std::cout << "Received request:\n" << request.substr(0, 200) << "...\n";

    // Парсим Host
    size_t host_pos = request.find("Host: ");
    if (host_pos == std::string::npos) {
        std::cerr << "Error: Host header not found\n";
        return;
    }

    host_pos += 6; // длина "Host: "
    size_t host_end = request.find("\r\n", host_pos);
    std::string host = request.substr(host_pos, host_end - host_pos);

    target_host_ = host;
    target_port_ = 80;

    size_t port_pos = host.find(":");
    if (port_pos != std::string::npos) {
        target_port_ = std::stoi(host.substr(port_pos + 1));
        target_host_ = host.substr(0, port_pos);
    }

    client_bytes_transferred_ = bytes_transferred;

    connect_to_server();
}

void ProxySession::connect_to_server() {
    tcp::resolver resolver(server_sock_.get_executor());
    boost::asio::connect(server_sock_, resolver.resolve(target_host_, std::to_string(target_port_)));

    boost::asio::async_write(server_sock_, boost::asio::buffer(client_buffer_, client_bytes_transferred_),
        [this](const boost::system::error_code& error, std::size_t) {
            handle_server_write(error, 0); // Передаем 0, т.к. данные уже отправлены
        });
}

void ProxySession::handle_server_connect(const boost::system::error_code& error) {
    if (error) {
        std::cerr << "Connection to server failed: " << error.message() << "\n";
        return;
    }

    boost::asio::async_write(server_sock_, boost::asio::buffer(client_buffer_, client_bytes_transferred_),
        [this](const boost::system::error_code& error, std::size_t) {
            handle_server_write(error, 0);
        });
}

void ProxySession::handle_server_write(const boost::system::error_code& error, std::size_t) {
    if (error) return;
    read_from_server();
}

void ProxySession::read_from_server() {
    server_sock_.async_read_some(boost::asio::buffer(server_buffer_, buffer_size),
        [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
            handle_server_read(error, bytes_transferred);
        });
}

void ProxySession::handle_server_read(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (error) return;

    boost::asio::async_write(client_sock_, boost::asio::buffer(server_buffer_, bytes_transferred),
        [this](const boost::system::error_code& error, std::size_t) {
            write_to_client(error, 0);
        });
}

void ProxySession::write_to_client(const boost::system::error_code& error, std::size_t) {
    if (error) return;
    read_from_server();
}

// Server Impl

ProxyServer::ProxyServer(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), io_context_(io_context) {
    start_accept();
}

void ProxyServer::start_accept() {
    ProxySession* new_session = new ProxySession(io_context_);
    acceptor_.async_accept(new_session->client_socket(),
        [this, new_session](const boost::system::error_code& error) {
            handle_accept(new_session, error);
        });
}

void ProxyServer::handle_accept(ProxySession* new_session, const boost::system::error_code& error) {
    if (!error) {
        new_session->start();
    }
    else {
        delete new_session;
    }

    start_accept();
}