#include <exception>
#include <iterator>
#include "../include/socket.hpp"

namespace sockets {
    Socket::Socket(const int domain, const int type, const int protocol) 
        : domain(domain), opt(1), address_len(sizeof(address)) {
        this->fd = socket(domain, type, protocol);
        if (this->fd < 0)
            throw std::runtime_error("Socket: socket creation failed");
    }

    void Socket::bind(const std::string host, int port) {
        if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &this->opt, sizeof(this->opt)))
            throw std::runtime_error("Socket: binding failed [setsockopt]");

        this->address.sin_family = this->domain;
        this->address.sin_addr.s_addr = inet_addr(host.c_str());
        this->address.sin_port = htons(port);

        if (::bind(this->fd, (sockaddr_t*)&this->address, sizeof(this->address)) < 0)
            throw std::runtime_error("Socket: binding failed");
    }

    void Socket::listen(int backlog) {
        if (::listen(this->fd, backlog) < 0)
            throw std::runtime_error("Socket: failed to listen");
    }

    void Socket::accept() {
        this->sock = ::accept(this->fd, (sockaddr_t*)&this->address, &this->address_len);
        if (this->sock < 0)
            throw std::runtime_error("Socket: accept failed");
    }

    void Socket::connect(const std::string host, int port) {
        this->address.sin_family = this->domain;
        this->address.sin_port = htons(port);
        if (inet_pton(this->domain, host.c_str(), &this->address.sin_addr) <= 0)
            throw std::runtime_error("Socket: Invalid address or address not supported");

        this->sock = ::connect(this->fd, (sockaddr_t*)&this->address, sizeof(this->address));
        if (this->sock < 0)
            throw std::runtime_error("Socket: Connection failed");
    }

    std::string Socket::recv(const ssize_t buffer_size) {
        if (buffer_size > BUFSIZ)
            throw std::length_error("Socket: Buffer size exceeds maximum limit");

        char buffer[buffer_size] = {0};
        ssize_t bytes_read = (this->sock > 0)
            ? ::read(this->sock, buffer, buffer_size) // receive from client
            : ::read(this->fd, buffer, buffer_size);  // receive from server
        if (bytes_read >= 0)
            buffer[bytes_read] = '\0';
        return std::string(buffer);
    }

    ssize_t Socket::send(const std::string message) {
        return (this->sock > 0)
            ? ::send(this->sock, message.c_str(), message.size(), 0) // send to client
            : ::send(this->fd, message.c_str(), message.size(), 0);  // send to server
    }

    Socket::~Socket() {
        ::close(this->fd);
    }
}
