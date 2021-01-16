#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <unistd.h>

#define MESSAGE_SIZE 2048

void *receive_m(void *arguments);
void *send_m(void *arguments);
void connect_to_peer(struct sockaddr_in *peer_address);
void clear_console();

struct args_struct {
  int *network_socket;
  struct sockaddr_in *peer_address;
};

int main(int argc, char **argv)
{
  pthread_t sender, receiver;

  int network_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (network_socket < 0)
  {
    printf("Failed to create socket");
    exit(1);
  }

  struct sockaddr_in server_address, peer_address;
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = 0;

  int connection = bind(network_socket, (struct sockaddr *)&server_address, sizeof(server_address));
  if (connection < 0)
  {
    printf("Failed to bind socket with the server address");
    exit(0);
  }

  socklen_t len = sizeof(server_address);
  getsockname(network_socket, (struct sockaddr *)&server_address, &len);

  clear_console();

  char ip[15];
  inet_ntop(AF_INET, &server_address.sin_addr, ip, sizeof(ip));
  printf("Your address:\t%s:%d\n\n", ip, ntohs(server_address.sin_port));

  connect_to_peer(&peer_address);

  struct args_struct p_thread_args;
  p_thread_args.network_socket = &network_socket;
  p_thread_args.peer_address = &peer_address;

  pthread_create(&sender, NULL, send_m, (void *)&p_thread_args);
  pthread_create(&receiver, NULL, receive_m, (void *)&p_thread_args);

  pthread_join(sender, NULL);
  pthread_join(receiver, NULL);

  close(network_socket);

  return 0;
}

void connect_to_peer(struct sockaddr_in *peer_address)
{
  char ip[15] = "0.0.0.0";
  char port[7];

  // printf("Peer ip: \t");
  // scanf("%s", ip);

  printf("Peer port: \t");
  fgets(port, 7, stdin);
  port[strlen(port) - 1] = '\0';

  peer_address->sin_family = AF_INET;
  peer_address->sin_addr.s_addr = inet_addr(ip);
  peer_address->sin_port = htons(atoi(port));
}

void *receive_m(void *arguments)
{
  struct args_struct *args = arguments;

  while (1)
  {
    char message[MESSAGE_SIZE];
    unsigned int peer_struct_length = sizeof(*args->peer_address);
    recvfrom(*args->network_socket, message, sizeof(message), 0, (struct sockaddr *)args->peer_address, &peer_struct_length);

    printf("%s:%i:\t%s\n", inet_ntoa(args->peer_address->sin_addr), ntohs(args->peer_address->sin_port), message);
    memset(message, 0, sizeof message);
  }
}

void *send_m(void *arguments)
{
  struct args_struct *args = arguments;

  char message[MESSAGE_SIZE] = "";
  while (1)
  {
    fgets(message, sizeof(message), stdin);
    message[strlen(message) - 1] = '\0';

    if (message[0] == '\0')
    {
      continue;
    }

    unsigned int peer_struct_length = sizeof(*args->peer_address);
    int res = sendto(*args->network_socket, message, strlen(message), 0, (struct sockaddr *)args->peer_address, peer_struct_length);
    if (res < 0)
    {
      printf("Unable to send message\n");
      exit(-1);
    }

    memset(message, 0, sizeof(message));
  }
}

void clear_console()
{
  printf("\033[2J\033[H");
  printf("\n");
}
