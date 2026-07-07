#include "server.h"
#include <pthread.h>


int main(void) {
    int sockfd = create_server_socket();

    struct sockaddr_storage their_addr;
    socklen_t sin_size; 
    int new_fd;

    char s[INET6_ADDRSTRLEN];

    while(1) {
        new_fd = accept_client(sockfd, &their_addr, &sin_size); 
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof(s));

        int *client_fd = malloc(sizeof(int));
        *client_fd = new_fd;
        pthread_t t1;
        pthread_create(&t1, NULL, &handle_client, client_fd);
        pthread_detach(t1);
    }

    return 0;
}
