#include "client_list.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void push_back(ClientList *head, time_t time, char ip[INET6_ADDRSTRLEN], int fd) {
    Node *new_node = malloc(sizeof(Node));
    if (!new_node) {
        perror("malloc");
        exit(1);
    }
    new_node->next = NULL;
    new_node->fd = fd;
    new_node->time = time;
    snprintf(new_node->ip, sizeof(new_node->ip), "%s", ip);

    if (head->head == NULL) {
        head->head = new_node;
    }
    else {
        Node *curr = head->head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = new_node;
    }
    head->size++;
    new_node->count = head->size;
}

void remove_client(ClientList *head, int fd) {
    Node *current = head->head; 
    Node *prev = NULL;
    while (current != NULL && current->fd != fd) {
        prev = current;
        current = current->next;
    }
    if (!current) {
        printf("There is no any client with fd: %d.\n", fd);
        return;
    }
    if (!prev) {
        head->head = current->next;
        head->size--;
        free(current);
    }
    else {
        prev->next = current->next;
        head->size--;
        free(current);
    }
}

Node *find_client(ClientList *head, char ip[INET6_ADDRSTRLEN]) {
    Node *current = head->head;
    while (current != NULL) {
        if (!strcmp(current->ip, ip))
            return current;
        current = current->next;
    }
    printf("There is no any client with ip: %s\n", ip);
    return NULL;
}

void print_list(ClientList *head) {
    Node *current = head->head;
    if (!current) {
        printf("List is empty.\n");
        return;
    }
    while (current != NULL) {
        printf("Client: #%zu | ip: %s | fd: %d\n",
               current->count, current->ip, current->fd);
        current = current->next;
    }
}

void free_list(ClientList *head) {
    Node *current = head->head;
    Node *prev = NULL;
    while (current != NULL) {
        prev = current;
        current = current->next;
        free(prev);
    }
    head->head = NULL;
    head->size = 0;
}
