#include "httpparse.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>

httpparser::httpparser(const std::string& request, bool req) : request(request) {
    if(req){
        parse_request(request);
    }else{
        parse_response(request);
    }
}

std::string httpparser::update_version(std::string request){
    size_t pos = request.find("HTTP/");
    if(pos!=std::string::npos){
        size_t endPos = request.find("\r\n", pos);
        if (endPos != std::string::npos) {
            // Заменить версию на HTTP/1.0
            request.replace(pos, endPos - pos, "HTTP/1.0");
        }
    }
    return request;
}

void httpparser::parse_request(const std::string& request) {
    std::istringstream iss(request);
    std::string line;

    // Parse the request line
    std::getline(iss, line);
    std::istringstream requestLine(line);
    requestLine >> method >> uri;

    // Default port is 80 for HTTP
    port = 80;

    // Parse headers
    while (std::getline(iss, line) && line != "\r\n") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2);
            headers[key] = value;

            if (key == "Host") {
                size_t portPos = value.find(':');
                if (portPos != std::string::npos) {
                    host = value.substr(0, portPos);
                    port = std::stoi(value.substr(portPos + 1));
                } else {
                    host = value;
                }
            }
        }
    }

    if (uri.find("http://") == 0) {
        uri = uri.substr(7); // Remove "http://"
        size_t pathPos = uri.find('/');
        if (pathPos != std::string::npos) {
            host = uri.substr(0, pathPos);
            // uri = uri.substr(pathPos);
        } else {
            host = uri;
            // uri = "/";
        }
    } else if (uri.find("https://") == 0) {
        uri = uri.substr(8); // Remove "https://"
        size_t pathPos = uri.find('/');
        if (pathPos != std::string::npos) {
            host = uri.substr(0, pathPos);
            // uri = uri.substr(pathPos);
        } else {
            host = uri;
            // uri = "/";
        }
        port = 443;
    }
}

void httpparser::parse_response(const std::string& response) {
    std::istringstream iss(response);
    std::string line;

    // Parse the status line
    std::getline(iss, line);
    std::istringstream statusLine(line);
    std::string httpVersion;
    statusLine >> httpVersion >> status_code;

    // Parse headers
    while (std::getline(iss, line) && line != "\r\n") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2);
            headers[key] = value;
        }
    }

    // for (const auto& header : headers) {
    //     std::cout << header.first << ": " << header.second << "\r\n";
    // }
}

int httpparser::get_status_code() {
    return status_code;
}

std::string httpparser::get_method() const {
    return method;
}

std::string httpparser::get_uri() const {
    return uri;
}

int httpparser::get_port() const {
    return port;
}

std::string httpparser::get_host() const {
    return host;
}

std::string httpparser::get_header(const std::string& key) const {
    auto it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}

std::map<std::string, std::string> httpparser::get_headers() const {
    return headers;
}

void httpparser::set_url(const std::string& new_url) {
    uri = new_url;
}

void httpparser::update_request_line() {
    if (uri.find("http://") == 0) {
        size_t pathPos = uri.substr(7).find('/');
        if (pathPos != std::string::npos) {
            host = uri.substr(7, pathPos);
        } else {
            host = uri.substr(7);
        }
        port = 80;
    } else if (uri.find("https://") == 0) {
        size_t pathPos = uri.substr(8).find('/');
        if (pathPos != std::string::npos) {
            host = uri.substr(8, pathPos);
        } else {
            host = uri.substr(8);
        }
        port = 443;
    }
    headers["Host"] = host;
}

std::string httpparser::get_request() const {
    return request;
}