#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT "3490"
#define BACKLOG 10

void *get_in_addr(struct sockaddr *sa);
void *handle_client(void *arg);
int accept_client(int sockfd, struct sockaddr_storage *their_addr, socklen_t *sin_size);
int create_server_socket();
void show_client();
