#pragma once

#include <exception>
#include <string>
#include <utility>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct sockaddr_in sockaddr_in_t;
typedef struct sockaddr sockaddr_t;

namespace sockets {
    class Socket {
   private:
        int fd;
        int sock;
        int domain;
        int opt;
        sockaddr_in_t address;
        socklen_t address_len;

        char buffer[BUFSIZ];
    public:
        Socket(const int domain, const int type, const int protocol = 0);
        ~Socket();

        void bind(const std::string host, int port);
        void listen(int backlog);
        void accept();
        void connect(const std::string host, int port);

        std::string recv(const ssize_t buffer_size=BUFSIZ);
        ssize_t send(const std::string message);
    };
};
