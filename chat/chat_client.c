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

#define MAXDATASIZE 100

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        printf("usage: client host port");
        exit(1);
    }

    struct addrinfo hints, *servinfo, *p;
    int sockfd, numbytes, rv;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    char buf[MAXDATASIZE];

    // set the hints for getaddrinfo
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s, sizeof s);
        printf("client: attempting connection to %s\n", s);

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s, sizeof s);
    printf("client: connected to %s\n", s);

    if (!fork()) {
        // child process receives messages
        while (true) {
            if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("client: recv");
                exit(1);
            }
            if (numbytes == 0) continue;

            buf[numbytes] = '\0';

            printf("%s", buf);

            if (buf[0] == 'b' && buf[1] == 'y' && buf[2] == 'e') {
                close(sockfd);
                break;
            }
        }
        exit(0);
    }

    // parent process sends messages

    while (true) {
        char message[MAXDATASIZE];

        fgets(message, MAXDATASIZE, stdin);

        if (send(sockfd, message, strlen(message), 0) == -1)
            perror("send");

        if (message[0] == 'b' && message[1] == 'y' && message[2] == 'e') break;
    }

    close(sockfd);

    return 0;
}
