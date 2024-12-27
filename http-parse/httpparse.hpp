#pragma once

#include <string>
#include <map>

class httpparser {
public:
    httpparser() = default;
    httpparser(const std::string& request, bool req);

    std::string get_method() const;
    std::string get_uri() const;
    int get_port() const;
    std::string get_host() const;
    std::string get_header(const std::string& key) const;
    std::map<std::string, std::string> get_headers() const;
    int get_status_code();
    std::string get_request() const;
    void set_url(const std::string& new_url);
    void update_request_line();

    static std::string update_version(std::string request); 

private:
    void parse_request(const std::string& request);
    void parse_response(const std::string& response);
    std::string request;
    std::string method;
    std::string uri;
    std::string host;
    int port;
    int status_code;
    std::map<std::string, std::string> headers;
};
