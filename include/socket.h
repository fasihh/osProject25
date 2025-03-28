#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

typedef struct socket_address
{
  char *host;
  int port;
} socket_address;

typedef struct socket
{
  int fd;
  int sock;
  int domain;
  int opt;
  struct sockaddr_in address;
  socklen_t address_len;
} socket_t;

socket_t *socket_create(int domain, int type, int protocol);
socket_t *socket_create_from_fd(int sock, int domain, struct sockaddr_in address);
void socket_destroy(socket_t *sock);

int socket_bind(socket_t *sock, const char *host, int port);
int socket_listen(socket_t *sock, int backlog);
socket_t *socket_accept(socket_t *sock, socket_address *client_addr);
int socket_connect(socket_t *sock, const char *host, int port);

char *socket_recv(socket_t *sock, size_t buffer_size, ssize_t *bytes_read);
ssize_t socket_send(socket_t *sock, const char *message, size_t length);

int socket_set_non_blocking(socket_t *sock, int status);
int socket_get_fd(const socket_t *sock);
void socket_close(socket_t *sock);

#endif
