#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <vector.h>
#include "socket.h"

#define USERNAME_MAX 32

typedef struct client_info {
  socket_t *socket;
  socket_address address;
  char username[USERNAME_MAX];
} client_info;

VECTOR_DEFINE(clients);

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void remove_client(socket_address *client_addr) {
  pthread_mutex_lock(&client_mutex);
  for (size_t i = 0; i < clients.vector_list.total; i++) {
    client_info *client = clients.pf_vector_get(&clients, i);
    if (client->address.port == client_addr->port &&
        strcmp(client->address.host, client_addr->host) == 0) {
      clients.pf_vector_delete(&clients, i);
      break;
    }
  }
  pthread_mutex_unlock(&client_mutex);
}

client_info *find_client_by_username(const char *username) {
  pthread_mutex_lock(&client_mutex);
  for (size_t i = 0; i < clients.vector_list.total; i++) {
    client_info *client = clients.pf_vector_get(&clients, i);
    if (strcmp(client->username, username) == 0) {
      pthread_mutex_unlock(&client_mutex);
      return client;
    }
  }
  pthread_mutex_unlock(&client_mutex);
  return NULL;
}

void broadcast(const char *message, const char *sender_name, socket_address *sender_addr) {
  pthread_mutex_lock(&client_mutex);

  char formatted_msg[1024];
  snprintf(formatted_msg, sizeof(formatted_msg), "[%s] %s", sender_name, message);

  for (size_t i = 0; i < clients.vector_list.total; i++) {
    client_info *client = clients.pf_vector_get(&clients, i);
    if (client->address.port == sender_addr->port &&
        strcmp(client->address.host, sender_addr->host) == 0)
      continue;

    socket_send(client->socket, formatted_msg, strlen(formatted_msg));
  }

  pthread_mutex_unlock(&client_mutex);
}

void *handle_client(void *arg) {
  client_info *client = (client_info *)arg;
  socket_t *client_sock = client->socket;
  socket_address client_addr = client->address;

  ssize_t name_len;
  char *username = socket_recv(client_sock, USERNAME_MAX, &name_len);

  if (!username || name_len <= 0) {
    fprintf(stderr, "Failed to receive username from %s:%d\n", client_addr.host, client_addr.port);
    socket_destroy(client_sock);
    free(client_addr.host);
    free(client);
    return NULL;
  }

  strncpy(client->username, username, USERNAME_MAX);
  client->username[USERNAME_MAX - 1] = '\0';
  free(username);

  // ✅ Add to clients list after username is set
  pthread_mutex_lock(&client_mutex);
  clients.pf_vector_add(&clients, client, sizeof(client_info));
  pthread_mutex_unlock(&client_mutex);

  printf("%s (%s:%d) joined.\n", client->username, client_addr.host, client_addr.port);

  const char *welcome = "Welcome to the chat!";
  socket_send(client_sock, welcome, strlen(welcome));

  while (1) {
    ssize_t bytes_read;
    char *response = socket_recv(client_sock, 1024, &bytes_read);

    if (!response || bytes_read <= 0) {
      free(response);
      break;
    }

    if (strcmp(response, "bye") == 0) {
      char left_msg[128];
      snprintf(left_msg, sizeof(left_msg), "%s has left the chat.", client->username);
      broadcast(left_msg, "Server", &client->address);
      free(response);
      break;
    }

    if (response[0] == '\\') {
      char *second_backslash = strchr(response + 1, '\\');
      if (second_backslash) {
        size_t username_len = second_backslash - (response + 1);
        char target_username[USERNAME_MAX];
        strncpy(target_username, response + 1, username_len);
        target_username[username_len] = '\0';

        const char *private_msg = second_backslash + 1;
        client_info *target_client = find_client_by_username(target_username);

        if (target_client) {
          char formatted_msg[1024];
          snprintf(formatted_msg, sizeof(formatted_msg), "[PM from %s] %s", client->username, private_msg);
          socket_send(target_client->socket, formatted_msg, strlen(formatted_msg));
        } else {
          const char *error_msg = "User not found.";
          socket_send(client_sock, error_msg, strlen(error_msg));
        }
      } else {
        const char *error_msg = "Invalid private message format. Use \\username\\message";
        socket_send(client_sock, error_msg, strlen(error_msg));
      }
    } else {
      printf("[%s] %s\n", client->username, response);
      broadcast(response, client->username, &client->address);
    }

    free(response);
  }

  printf("%s disconnected\n", client->username);
  remove_client(&client_addr);
  socket_destroy(client_sock);
  free(client->address.host);
  free(client);

  return NULL;
}

int main() {
  vector_init(&clients);

  socket_t *server = socket_create(AF_INET, SOCK_STREAM, 0);
  if (!server) {
    fprintf(stderr, "Failed to create socket\n");
    return EXIT_FAILURE;
  }

  if (socket_bind(server, "127.0.0.1", 8080) < 0) {
    fprintf(stderr, "Failed to bind\n");
    socket_destroy(server);
    return EXIT_FAILURE;
  }

  if (socket_listen(server, 5) < 0) {
    fprintf(stderr, "Failed to listen\n");
    socket_destroy(server);
    return EXIT_FAILURE;
  }

  while (1) {
    socket_address client_addr;
    socket_t *client_socket = socket_accept(server, &client_addr);

    if (!client_socket) {
      fprintf(stderr, "Failed to accept connection\n");
      continue;
    }

    client_info *client = malloc(sizeof(client_info));
    client->socket = client_socket;
    client->address = client_addr;

    pthread_t thread;
    if (pthread_create(&thread, NULL, handle_client, client) != 0) {
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
