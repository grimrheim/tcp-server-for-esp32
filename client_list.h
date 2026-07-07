#pragma once
#include <netinet/in.h>
#include <time.h>

typedef struct Node {
    int fd;
    char ip[INET6_ADDRSTRLEN];
    time_t time;
    size_t count;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    size_t size;
} ClientList;

void push_back(ClientList *head, time_t time, char ip[INET6_ADDRSTRLEN], int fd);
void remove_client(ClientList *head, int fd);
Node *find_client(ClientList *head, char ip[INET6_ADDRSTRLEN]);
void free_list(ClientList *head);
void print_list(ClientList *head);
