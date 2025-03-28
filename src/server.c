#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector.h>
#include "socket.h"

typedef struct client_info
{
  socket_t *socket;
  socket_address address;
} client_info;

VECTOR_DEFINE(clients);

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void remove_client(socket_address *client_addr)
{
  pthread_mutex_lock(&client_mutex);

  for (size_t i = 0; i < clients.vector_list.total; i++)
  {
    client_info *client = clients.pf_vector_get(&clients, i);
    if (client->address.port == client_addr->port &&
        strcmp(client->address.host, client_addr->host) == 0)
    {
      clients.pf_vector_delete(&clients, i);
      break;
    }
  }

  pthread_mutex_unlock(&client_mutex);
}

void broadcast(const char *message, size_t msg_len, socket_address *sender_addr)
{
  pthread_mutex_lock(&client_mutex);

  for (size_t i = 0; i < clients.vector_list.total; i++)
  {
    client_info *client = clients.pf_vector_get(&clients, i);
    if (client->address.port == sender_addr->port &&
      strcmp(client->address.host, sender_addr->host) == 0)
    {
      continue;
    }
    socket_send(client->socket, message, msg_len);
  }

  pthread_mutex_unlock(&client_mutex);
}

void *handle_client(void *arg)
{
  client_info *client = (client_info *)arg;
  socket_t *client_sock = client->socket;
  socket_address client_addr = client->address;

  const char *welcome = "Welcome!";
  socket_send(client_sock, welcome, strlen(welcome));

  while (1)
  {
    ssize_t bytes_read;
    char *response = socket_recv(client_sock, 1024, &bytes_read);

    if (!response || bytes_read <= 0)
    {
      free(response);
      break;
    }

    printf("%s:%d = %s\n", client_addr.host, client_addr.port, response);
    broadcast(response, bytes_read, &client_addr);
    free(response);
  }

  remove_client(&client_addr);
  printf("%s:%d disconnected\n", client_addr.host, client_addr.port);
  socket_destroy(client_sock);
  free(client->address.host);
  free(client);

  return NULL;
}

int main()
{
  vector_init(&clients);

  socket_t *server = socket_create(AF_INET, SOCK_STREAM, 0);
  if (!server)
  {
    fprintf(stderr, "Failed to create socket\n");
    return EXIT_FAILURE;
  }

  if (socket_bind(server, "127.0.0.1", 8080) < 0)
  {
    fprintf(stderr, "Failed to bind\n");
    socket_destroy(server);
    return EXIT_FAILURE;
  }

  if (socket_listen(server, 5) < 0)
  {
    fprintf(stderr, "Failed to listen\n");
    socket_destroy(server);
    return EXIT_FAILURE;
  }

  while (1)
  {
    socket_address client_addr;
    socket_t *client_socket = socket_accept(server, &client_addr);

    if (!client_socket)
    {
      fprintf(stderr, "Failed to accept connection\n");
      continue;
    }

    printf("%s:%d connected\n", client_addr.host, client_addr.port);

    client_info *client = malloc(sizeof(client_info));
    client->socket = client_socket;
    client->address = client_addr;

    pthread_mutex_lock(&client_mutex);
    clients.pf_vector_add(&clients, client, sizeof(client_info));
    pthread_mutex_unlock(&client_mutex);

    pthread_t thread;
    if (pthread_create(&thread, NULL, handle_client, client) != 0)
    {
      fprintf(stderr, "Failed to create thread\n");
      socket_destroy(client_socket);
      free(client->address.host);
      free(client);
      continue;
    }
    pthread_detach(thread);
  }

  socket_destroy(server);
  clients.pf_vector_free(&clients);
  pthread_mutex_destroy(&client_mutex);

  return 0;
}