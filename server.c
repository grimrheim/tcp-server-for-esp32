// POSIX не является стандартом C. 
// Чтобы подключить расширения для сети, добавляем _GNU_SOURCE
#define _GNU_SOURCE
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include "server.h"
#include "client_list.h"

#define PORT "3490"
#define BACKLOG 10

static ClientList list;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *handle_client(void *arg) {
    int *fd = (int *)arg;
    int client_fd = *fd;
    free(fd);
    time_t t = time(NULL);

    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    char s[INET6_ADDRSTRLEN];
    /*
     * 1. getpeername() - ядро пишет бинарный адрес fd в addr.
     * 2. get_in_addr() - возвращает указатель на нужное поле IPv4/IPv6.
     * 3. inet_ntop()   - преобразует бинарный адрес в читаемый вид, записывает в s.
    */
    if (getpeername(client_fd, (struct sockaddr *)&addr, &len) == -1)
        perror("getpeername");
    inet_ntop(addr.ss_family,
              get_in_addr((struct sockaddr *)&addr),
              s, sizeof(s));
 
    pthread_mutex_lock(&mutex);
    push_back(&list, t, s, client_fd);
    print_list(&list);
    pthread_mutex_unlock(&mutex);

    if (send(client_fd, "Hello, world", 13, 0) == -1)
        perror("send");
    close(client_fd);
    remove_client(&list, client_fd);
    return NULL;
}

int create_server_socket() {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv = getaddrinfo(NULL, PORT, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
        exit(1);
    }
    printf("server: waiting for connections...\n");
    return sockfd;
}

int accept_client(int sockfd, struct sockaddr_storage *their_addr, socklen_t *sin_size) {
    *sin_size = sizeof(*their_addr);
    int new_fd = accept(sockfd,
                        (struct sockaddr *)their_addr,
                        sin_size);

    return new_fd;
}

