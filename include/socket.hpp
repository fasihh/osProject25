#pragma once

#ifndef SOCKET_H
#define SOCKET_H

#include <iostream>
#include <string>
#include <utility>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <socket-address.hpp>

namespace os_sock {
    typedef struct sockaddr_in sockaddr_in_t;
    typedef struct sockaddr sockaddr_t;

    class Socket {
    private:
        int fd = -1;
        int sock = -1;
        int domain;
        int opt;
        sockaddr_in_t address;
        socklen_t address_len;
    public:
        Socket(const int domain, const int type, const int protocol = 0);
        Socket(int sock, int domain, sockaddr_in_t address);
        ~Socket();

        void bind(const std::string host, int port);
        void listen(int backlog);
        std::pair<Socket, SocketAddress> accept();
        void connect(const std::string& host, int port);

        std::string recv(const ssize_t buffer_size=BUFSIZ);
        ssize_t send(const std::string& message);

        void set_non_blocking(bool status);
        int get_fd() const;
        void close();
    };
};

#endif
