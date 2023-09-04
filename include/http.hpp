#ifndef HTTP
#define HTTP

#include "sock.hpp"
#include <functional>

class HttpRequest {
    explicit HttpRequest(std::pair<std::string, std::string> url, std::vector<uint8_t> bytes)
        noexcept : url(url), bytes(bytes) {}
public:
    std::pair<std::string, std::string> url;
    const std::vector<uint8_t> bytes;

    static HttpRequest from(const tcp::Socket& client);
};

class HttpResponse {
public:
    const uint16_t status;
    const std::vector<uint8_t> bytes;

    HttpResponse(uint16_t status) noexcept : status(status) {}

    HttpResponse(uint16_t status, const std::string& content) noexcept
        : status(status), bytes(content.begin(), content.end()) {}

    HttpResponse(uint16_t status, std::vector<uint8_t> bytes) noexcept
        : status(status), bytes(bytes) {}

    void respond(const tcp::Socket& client) const;
};

class Router {
    std::vector<std::function<HttpResponse(HttpRequest)>> handlers;
    std::vector<std::pair<std::string, std::string>> urls;

    bool does_url_match(const std::string& base, const std::string& url) const noexcept;
public:
    void get(std::string url, std::function<HttpResponse(HttpRequest)> handler) noexcept;
    void post(std::string url, std::function<HttpResponse(HttpRequest)> handler) noexcept;

    void serve_directory(std::string path);

    void run(uint16_t port) const noexcept;
};

#endif /* HTTP */
