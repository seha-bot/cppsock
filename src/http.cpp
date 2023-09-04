#include "http.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <unistd.h>

static std::vector<std::string> split(std::string str, std::string delim) {
    std::vector<std::string> tokens;
    std::string::size_type next = 0, last = 0;
    while ((next = str.find(delim, next)) != std::string::npos) {
        tokens.push_back(str.substr(last, next - last));
        next += delim.size();
        last = next;
    }
    if (next == std::string::npos) {
        tokens.push_back(str.substr(last));
    }
    return tokens;
}

HttpRequest HttpRequest::from(const tcp::Socket& client) {
    std::vector<uint8_t> buf;
    while (true) {
        const auto data = client.read(1024, std::chrono::milliseconds(100));
        if (data.empty()) {
            break;
        }
        buf.insert(buf.end(), data.begin(), data.end());
        if (buf.size() >= 10240) {
            throw std::runtime_error("The request is too large");
        }
    }

    const auto pat = { '\r', '\n', '\r', '\n' };

    const auto first_split = std::search(buf.begin(), buf.end(), pat.begin(), pat.end() - 2);
    if (first_split == buf.end()) {
        throw std::runtime_error("Bad protocol");
    }

    const auto data = split(std::string(buf.begin(), first_split), " ");
    if (data.size() != 3) {
        std::runtime_error("Bad protocol");
    }

    const auto head_end = std::search(first_split, buf.end(), pat.begin(), pat.end());
    if (head_end == buf.end()) {
        throw std::runtime_error("Bad protocol");
    }

    return HttpRequest({ data[0], data[1] }, {head_end + 4, buf.end()});
}

void HttpResponse::respond(const tcp::Socket& client) const {
    const auto header = "HTTP/1.0 " + std::to_string(status) + "\r\n\r\n";
    std::vector<uint8_t> response(header.begin(), header.end());
    response.insert(response.end(), bytes.begin(), bytes.end());
    client.write(response);
}

bool Router::does_url_match(const std::string& base, const std::string& url) const noexcept {
    if (base == url) {
        return 1;
    }

    std::string::size_type i = 0, j = 0;
    while (base[i] && url[j]) {
        if (base[i] == url[j]) {
            i++;
            j++;
        } else if (base[i] == '*') {
            i++;
            while (url[j] && url[j] != '/') {
                j++;
            }
        } else {
            break;
        }
    }

    return base[i] == url[j] || base[i] == '*';
}

void Router::get(std::string url, std::function<HttpResponse(HttpRequest)> handler) noexcept {
    urls.push_back({ "GET", url });
    handlers.push_back(handler);
}

void Router::post(std::string url, std::function<HttpResponse(HttpRequest)> handler) noexcept {
    urls.push_back({ "POST", url });
    handlers.push_back(handler);
}

void Router::serve_directory(std::string path) {
    for (const auto& item : std::filesystem::recursive_directory_iterator(path)) {
        if (item.is_directory()) {
            continue;
        }

        const auto file_path = item.path().string();
        auto url_path = file_path.substr(path.size());
        if (url_path.find("index.html") != std::string::npos) {
            url_path = url_path.substr(0, url_path.size() - 10);
        }
        urls.push_back({ "GET", url_path });
        handlers.push_back([file_path](HttpRequest req) {
            std::ifstream file(file_path);

            file.seekg(0, std::ios_base::end);
            const size_t file_size = file.tellg();
            file.seekg(0);

            std::string buf;
            buf.resize(file_size);

            file.read(&buf[0], file_size);

            return HttpResponse(200, buf);
        });
    }
}

void Router::run(uint16_t port) const noexcept {
    const auto server = tcp::Server(port);

    while (true) {
        try {
            const auto client = server.await_client();
            client.await_data();

            const auto req = HttpRequest::from(client);

            for (size_t i = 0; i < urls.size(); i++) {
                if (urls[i].first == req.url.first
                && does_url_match(urls[i].second, req.url.second)) {
                    const auto res = handlers[i](req);
                    res.respond(client);
                    break;
                }
            }
        } catch (std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}
