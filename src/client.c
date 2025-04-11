#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "socket.h"

#define BUFFER_SIZE 1024

pthread_mutex_t console_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile bool is_running = true;

typedef struct thread_args
{
  socket_t *socket;
} thread_args;

void *receive_messages(void *arg)
{
  thread_args *args = (thread_args *)arg;
  socket_t *client_socket = args->socket;

  while (is_running)
  {
    ssize_t bytes_read;
    char *response = socket_recv(client_socket, BUFFER_SIZE, &bytes_read);

    if (!response || bytes_read <= 0)
    {
      pthread_mutex_lock(&console_mutex);
      fprintf(stderr, "Server disconnected.\n");
      pthread_mutex_unlock(&console_mutex);
      is_running = false;
      free(response);
      break;
    }

    pthread_mutex_lock(&console_mutex);
    printf("%s\n", response);
    pthread_mutex_unlock(&console_mutex);
    free(response);
  }

  return NULL;
}

int main()
{
  socket_t *client_socket = socket_create(AF_INET, SOCK_STREAM, 0);
  if (!client_socket)
  {
    fprintf(stderr, "Failed to create socket\n");
    return EXIT_FAILURE;
  }

  if (socket_connect(client_socket, "127.0.0.1", 8080) < 0)
  {
    fprintf(stderr, "Failed to connect to server\n");
    socket_destroy(client_socket);
    return EXIT_FAILURE;
  }

  pthread_mutex_lock(&console_mutex);
  printf("Connected to the server!\n");
  pthread_mutex_unlock(&console_mutex);

  char username[32];
  printf("Enter your username: ");
  fgets(username, sizeof(username), stdin);
  username[strcspn(username, "\n")] = '\0'; // remove newline

  socket_send(client_socket, username, strlen(username));

  thread_args args = {
      .socket = client_socket};

  pthread_t listener_thread;
  if (pthread_create(&listener_thread, NULL, receive_messages, &args) != 0)
  {
    fprintf(stderr, "Failed to create listener thread\n");
    socket_destroy(client_socket);
    return EXIT_FAILURE;
  }

  char message[BUFFER_SIZE];
  while (is_running)
  {
    if (fgets(message, BUFFER_SIZE, stdin) == NULL)
    {
      break;
    }

    size_t len = strlen(message);
    if (len > 0 && message[len - 1] == '\n')
    {
      message[len - 1] = '\0';
      len--;
    }

    if (strcmp(message, "bye") == 0)
    {
      socket_send(client_socket, message, len);
      is_running = false;
      break;
    }

    if (socket_send(client_socket, message, len) < 0)
    {
      pthread_mutex_lock(&console_mutex);
      fprintf(stderr, "Failed to send message\n");
      pthread_mutex_unlock(&console_mutex);
      break;
    }
  }

  is_running = false;
  socket_close(client_socket);

  if (pthread_join(listener_thread, NULL) != 0)
  {
    fprintf(stderr, "Failed to join listener thread\n");
  }

  pthread_mutex_destroy(&console_mutex);
  socket_destroy(client_socket);

  return EXIT_SUCCESS;
}
