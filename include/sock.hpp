#ifndef SOCKS
#define SOCKS

#include <string>
#include <cstdint>
#include <vector>
#include <chrono>

namespace tcp {
    class Address {
    public:
        const std::string ip;
        const uint16_t port;

        Address(std::string ip, uint16_t port) noexcept
            : ip(ip), port(port) {}
    };

    class Socket {
    protected:
        const int fd;

        void connect(const Address& address) const;
        void listen(uint16_t port) const;
        Socket accept() const;

        explicit Socket(int fd) noexcept : fd(fd) {}
        explicit Socket();
    public:
        // TODO await_data is a temporary solution
        void await_data() const noexcept;
        std::vector<uint8_t> read(size_t n, std::chrono::milliseconds timeout) const;
        void write(const std::vector<uint8_t>& buf) const;

        Socket(const Socket&) = delete;
        Socket(Socket&& other) = default;

        Socket& operator=(const Socket& other) = delete;
        Socket& operator=(Socket&& other) = delete;

        ~Socket() noexcept;
    };

    class Client : public Socket {
    public:
        const Address& address;

        explicit Client(const Address& address);
    };

    class Server : public Socket {
    public:
        Socket await_client() const;

        explicit Server(uint16_t port);
    };
}

#endif /* SOCKS */
