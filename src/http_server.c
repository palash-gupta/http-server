#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <poll.h>

#define PORT "3490"
#define BACKLOG 10
#define MAX_MSG_LEN 1024

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 

int create_listener_socket() {
    int sockfd, rv, yes=1;
    struct addrinfo hints, *servinfo, *p;

    // initialize the hints addrinfo struct
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rv = getaddrinfo(nullptr, PORT, &hints, &servinfo);

    if (rv != 0) {
        fprintf(stderr, "httpserver: getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for (p = servinfo; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        
        if (sockfd == -1) {
            perror("httpserver: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("httpserver: setsockopt");
            return -1;
        }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("httpserver: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == nullptr) {
        fprintf(stderr, "httpserver: failed to bind\n");
        return -1;
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("httpserver: listen");
        return -1;
    }

    return sockfd;
}

void serve_new_client(int sockfd) {
    struct sockaddr_storage their_addr;
    socklen_t sin_size = sizeof(their_addr);

    int client_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

    char addr_string[INET6_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), addr_string, sizeof addr_string);

    if (client_fd == -1) {
        perror("httpserver: accept");
        fprintf(stderr, "httpserver: unable to assign socket to client %s\n", addr_string);
        exit(1);
    }

    printf("httpserver: received connection from %s\n", addr_string);

    char buf[MAX_MSG_LEN];
    recv(client_fd, buf, sizeof buf, 0);

    char payload[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: 47\r\n\r\n<html><body><h1>Hello World.</h1></body></html>\r\n";
    send(client_fd, payload, sizeof payload, 0);
    close(client_fd);

}

int main(void) {
    int sockfd = create_listener_socket();

    struct pollfd pfds[1];
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;

    while (true) {
        int num_events = poll(pfds, 1, -1);

        if (num_events) {
            serve_new_client(sockfd);
        }
    }

    close(sockfd);
    return 0;
}
