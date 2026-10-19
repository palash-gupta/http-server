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

#define PORT 3490
#define BACKLOG 10

void sigchld_handler(int s) {
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void) {

    // socket file descriptor for receiving incoming connections
    int sockfd;
    // fetch address info
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // tcp (SOCK_DGRAM would be udp)
    hints.ai_flag = AI_PASSIVE; // use own IP

    if ((rv = getaddrinfo(nullptr, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != nullptr, p = p->ai_next) {
        // create socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        // at the socket level, set reuse addr to yes
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        // bind to socket
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == nullptr) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // set the port to listening mode
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // set the handling of SIGCHLD
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("waiting for connections...\n");

    // main loop for accepting connections and spawning child processes
    struct sockaddr_storage their_addr; // connector's address info
    socklen_t sin_size;
    int new_fd;
    char addr_string[INET6_ADDRSTRLEN];
    while (1) {
        sin_size = sizeof their_addr;
        // create a new file descriptor for the connections first in the queue
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            exit(1);
        }


        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), addr_string, sizeof addr_string);
        printf("server: received connection from %s\n", s);

        if (!fork()) {
            // child process
            
            close(sockfd);

            int numbytes;
            char buf[MAXDATASIZE];

            struct pollfd pfds[1];
            pfds[0].fd = new_fd;
            pfds.events = POLLIN;

            while (1) {
                // poll the socket file descriptor indefinitely
                int num_events = poll(pfds, 1, -1);
                if (num_events == 0) {
                    fprintf(stderr, "Unexpected error occured while polling.");
                    exit(1);
                }
                else {


                }

            }
        }
    }


}
