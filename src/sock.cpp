#include <cstddef>
#include <stdexcept>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "sock.hpp"

void tcp::Socket::connect(const Address& address) const {
    throw std::runtime_error("Unimplemented");
}

void tcp::Socket::listen(uint16_t port) const {
    const int val = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) < 0) {
        throw std::runtime_error("Can't set SO_REUSEADDR");
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Can't bind socket");
    }

    if (::listen(fd, 10) < 0) {
        throw std::runtime_error("Can't listen socket");
    }
}

tcp::Socket tcp::Socket::accept() const {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    const int client_fd = ::accept(fd, (struct sockaddr *)&addr, &addr_len);
    if (client_fd < 0) {
        throw std::runtime_error("Can't accept connection");
    }

    return Socket(client_fd);
}

tcp::Socket::Socket() : fd(socket(PF_INET, SOCK_STREAM, 0)) {
    if (fd < 0) {
        throw std::runtime_error("Can't create a socket");
    }
}

void tcp::Socket::await_data() const noexcept {
    fd_set is_ready;
    FD_ZERO(&is_ready);
    FD_SET(fd, &is_ready);

    select(fd + 1, &is_ready, NULL, NULL, NULL);
}

std::vector<uint8_t> tcp::Socket::read(size_t n, std::chrono::milliseconds timeout) const {
    fd_set is_ready;
    FD_ZERO(&is_ready);
    FD_SET(fd, &is_ready);
    struct timeval timeout_val;
    timeout_val.tv_sec = 0;
    timeout_val.tv_usec = timeout.count();

    const int status = select(fd + 1, &is_ready, NULL, NULL, &timeout_val);
    if (status < 0) {
        throw std::runtime_error("Can't read data");
    } else if (status == 0) {
        return {};
    }

    std::vector<uint8_t> buf(n);
    const int size = recv(fd, buf.data(), n, 0);
    buf.resize(size);
    return buf;
}

void tcp::Socket::write(const std::vector<uint8_t>& buf) const {
    if (send(fd, buf.data(), buf.size(), 0) < 0) {
        throw std::runtime_error("Socket::write");
    }
}

tcp::Socket::~Socket() noexcept { close(fd); }

tcp::Client::Client(const Address& address) : address(address) { connect(address); }

tcp::Socket tcp::Server::await_client() const { return accept(); }

tcp::Server::Server(uint16_t port) { listen(port); }
