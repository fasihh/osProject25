#include <iostream>
#include <stdexcept>
#include <fcntl.h>
#include <socket.hpp>

namespace os_sock {
    Socket::Socket(const int domain, const int type, const int protocol) 
        : domain(domain), opt(1), address_len(sizeof(address)) {
        this->fd = socket(domain, type, protocol);
        if (this->fd < 0)
            throw std::runtime_error("Socket: Socket creation failed");
    }

    Socket::Socket(int sock, int domain, os_sock::sockaddr_in_t address)
        : sock(sock), domain(domain), address(address), address_len(sizeof(address)) {}

    void Socket::bind(const std::string host, int port) {
        if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &this->opt, sizeof(this->opt)))
            throw std::runtime_error("Socket: Binding failed [setsockopt]");

        this->address.sin_family = this->domain;
        this->address.sin_addr.s_addr = inet_addr(host.c_str());
        this->address.sin_port = htons(port);

        if (::bind(this->fd, (os_sock::sockaddr_t*)&this->address, sizeof(this->address)) < 0)
            throw std::runtime_error("Socket: Binding failed");
    }

    void Socket::listen(int backlog) {
        if (::listen(this->fd, backlog) < 0)
            throw std::runtime_error("Socket: Failed to listen");
    }

    std::pair<Socket, os_sock::addr_p_t> Socket::accept() {
        os_sock::sockaddr_in_t client_address;
        socklen_t client_len = sizeof(client_address);
        int client_fd = ::accept(this->fd, (os_sock::sockaddr_t*)&client_address, &client_len);

        if (client_fd < 0)
            throw std::runtime_error("Socket: Accept failed");

        char address_buffer[INET_ADDRSTRLEN];
        inet_ntop(this->domain, &(client_address.sin_addr), address_buffer, INET_ADDRSTRLEN);
        return {Socket(client_fd, this->domain, client_address), {std::string(address_buffer), client_address.sin_port}};
    }

    void Socket::connect(const std::string host, int port) {
        this->address.sin_family = this->domain;
        this->address.sin_port = htons(port);
        if (inet_pton(this->domain, host.c_str(), &this->address.sin_addr) <= 0)
            throw std::runtime_error("Socket: Invalid address or address not supported");

        this->sock = ::connect(this->fd, (os_sock::sockaddr_t*)&this->address, sizeof(this->address));
        if (this->sock < 0)
            throw std::runtime_error("Socket: Connection failed");
    }

    std::string Socket::recv(const ssize_t buffer_size) {
        if (buffer_size > BUFSIZ)
            throw std::length_error("Socket: Buffer size exceeds maximum limit");
    
        char buffer[buffer_size] = {0};
        ssize_t bytes_read = this->sock > 0
            ? ::read(this->sock, buffer, buffer_size) // receive from client
            : this->fd > 0
                ? ::read(this->fd, buffer, buffer_size)  // receive from server
                : -1;
        
        if (bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return "";
            throw std::runtime_error("Socket: Error receiving data");
        }
    
        if (bytes_read == 0)
            throw std::runtime_error("Socket: Connection closed by peer");
    
        return std::string(buffer, bytes_read);
    }
    

    ssize_t Socket::send(const std::string message) {
        if (this->sock > 0)
            return ::send(this->sock, message.c_str(), message.size(), 0); // send to client
        
        if (this->fd > 0) {
            ssize_t bytes_sent = ::send(this->fd, message.c_str(), message.size(), 0); // send to server
            if (bytes_sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
                return 0;
        }

        return -1;
    }

    void Socket::set_non_blocking(bool status) {
        int flags = ::fcntl(this->fd, F_GETFL, 0);
        if (flags < 0)
            throw std::runtime_error("Socket: Failed to fetch fd flags");
        
        if (status)
            flags |= O_NONBLOCK;
        else
            flags &= ~O_NONBLOCK;
        
        if (::fcntl(this->fd, F_SETFL, flags) < 0)
            throw std::runtime_error("Socket: failed to set fd flags");
    }

    void Socket::close() {
        if (this->fd != -1)
            ::close(this->fd);
    }

    Socket::~Socket() {
        this->close();
    }

    bool Socket::operator==(const Socket& obj) const {
        return obj.fd == this->fd || obj.sock == this->sock;
    }
}
