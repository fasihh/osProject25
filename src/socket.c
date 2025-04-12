#include "socket.h"

socket_t *socket_create(int domain, int type, int protocol)
{
  socket_t *sock = malloc(sizeof(socket_t));
  if (!sock)
    return NULL;

  sock->domain = domain;
  sock->opt = 1;
  sock->address_len = sizeof(sock->address);
  sock->fd = socket(domain, type, protocol);
  sock->sock = -1;

  if (sock->fd < 0)
  {
    free(sock);
    return NULL;
  }

  return sock;
}

socket_t *socket_create_from_fd(int sock_fd, int domain, struct sockaddr_in address)
{
  socket_t *sock = malloc(sizeof(socket_t));
  if (!sock)
    return NULL;

  sock->sock = sock_fd;
  sock->domain = domain;
  sock->address = address;
  sock->address_len = sizeof(address);
  sock->fd = -1;

  return sock;
}

int socket_bind(socket_t *sock, const char *host, int port)
{
  if (!sock)
    return -1;

  #ifdef SO_REUSEPORT
    if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &sock->opt, sizeof(sock->opt)) < 0)
  #else
    if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR | 15, &sock->opt, sizeof(sock->opt)) < 0)
  #endif
  {
    return -1;
  }

  sock->address.sin_family = sock->domain;
  sock->address.sin_addr.s_addr = inet_addr(host);
  sock->address.sin_port = htons(port);

  return bind(sock->fd, (struct sockaddr *)&sock->address, sizeof(sock->address));
}

int socket_listen(socket_t *sock, int backlog)
{
  if (!sock)
    return -1;
  return listen(sock->fd, backlog);
}

socket_t *socket_accept(socket_t *sock, socket_address *client_addr)
{
  if (!sock)
    return NULL;

  struct sockaddr_in client_address;
  socklen_t client_len = sizeof(client_address);
  int client_fd = accept(sock->fd, (struct sockaddr *)&client_address, &client_len);

  if (client_fd < 0)
    return NULL;

  if (client_addr)
  {
    char address_buffer[INET_ADDRSTRLEN];
    inet_ntop(sock->domain, &(client_address.sin_addr), address_buffer, INET_ADDRSTRLEN);
    client_addr->host = strdup(address_buffer);
    client_addr->port = ntohs(client_address.sin_port);
  }

  return socket_create_from_fd(client_fd, sock->domain, client_address);
}

int socket_connect(socket_t *sock, const char *host, int port)
{
  if (!sock)
    return -1;

  sock->address.sin_family = sock->domain;
  sock->address.sin_port = htons(port);

  if (inet_pton(sock->domain, host, &sock->address.sin_addr) <= 0)
    return -1;

  sock->sock = connect(sock->fd, (struct sockaddr *)&sock->address, sizeof(sock->address));
  return sock->sock;
}

char *socket_recv(socket_t *sock, size_t buffer_size, ssize_t *bytes_read)
{
  if (!sock || buffer_size <= 0 || buffer_size > BUFSIZ)
  {
    if (bytes_read)
      *bytes_read = -1;
    return NULL;
  }

  char *buffer = malloc(buffer_size);
  if (!buffer)
  {
    if (bytes_read)
      *bytes_read = -1;
    return NULL;
  }

  ssize_t received = sock->sock > 0 ? recv(sock->sock, buffer, buffer_size - 1, 0) : sock->fd > 0 ? recv(sock->fd, buffer, buffer_size - 1, 0)
                                                                                                  : -1;

  if (bytes_read)
    *bytes_read = received;

  if (received < 0)
  {
    free(buffer);
    return NULL;
  }

  buffer[received] = '\0';
  return buffer;
}

ssize_t socket_send(socket_t *sock, const char *message, size_t length)
{
  if (!sock || !message)
    return -1;

  if (sock->sock > 0)
    return send(sock->sock, message, length, 0);

  if (sock->fd > 0)
  {
    ssize_t bytes_sent = send(sock->fd, message, length, 0);
    if (bytes_sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
      return 0;
    return bytes_sent;
  }

  return -1;
}

int socket_set_non_blocking(socket_t *sock, int status)
{
  if (!sock)
    return -1;

  int flags = fcntl(sock->fd, F_GETFL, 0);
  if (flags < 0)
    return -1;

  flags = status ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
  return fcntl(sock->fd, F_SETFL, flags);
}

int socket_get_fd(const socket_t *sock)
{
  return sock ? sock->fd : -1;
}

void socket_close(socket_t *sock)
{
  if (sock)
  {
    if (sock->fd != -1)
      close(sock->fd);
    if (sock->sock != -1)
      close(sock->sock);
  }
}

void socket_destroy(socket_t *sock)
{
  if (sock)
  {
    socket_close(sock);
    free(sock);
  }
}