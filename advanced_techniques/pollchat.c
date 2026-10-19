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
#define MAX_MSG_LEN 256

// || fetch address info for own server
// || create a socket for listening for incoming connections
// accept incoming connections and assign them their own sockets
// have a poll_fds array to keep a list of all sockets that need to be polled
// in case of a successful poll, send the same data to all other sockets in poll_fds.

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
        fprintf(stderr, "pollchat: getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for (p = servinfo; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        
        if (sockfd == -1) {
            perror("pollchat: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("pollchat: setsockopt");
            return -1;
        }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("pollchat: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == nullptr) {
        fprintf(stderr, "pollchat: failed to bind\n");
        return -1;
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("pollchat: listen");
        return -1;
    }

    return sockfd;
}

void create_new_client(int listener, struct pollfd **pfds, int *fd_size, int *fd_count) {
    struct sockaddr_storage their_addr;
    socklen_t sin_size = sizeof(their_addr);

    int new_fd = accept(listener, (struct sockaddr *)&their_addr, &sin_size);

    char addr_string[INET6_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), addr_string, sizeof addr_string);

    if (new_fd == -1) {
        perror("pollchat: accept");
        fprintf(stderr, "pollchat: unable to assign socket to client %s\n", addr_string);
        return;
    }

    printf("pollchat: received connection from %s\n", addr_string);


    if (*fd_count == *fd_size) {
        *fd_size *= 2; // double the size of the pfds array when we run out of space
        *pfds = realloc(*pfds, sizeof(struct pollfd) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = new_fd;
    (*pfds)[*fd_count].events = POLLIN;
    (*pfds)[*fd_count].revents = 0;

    (*fd_count)++;
}

void process_hangup(struct pollfd **pfds, int *fd_size, int *fd_count, int i) {

    printf("pollchat: connection closed from socket %d\n", (*pfds)[i].fd);
    // close the socket file descriptor
    close((*pfds)[i].fd);

    // copy over the last person to this spot

    (*pfds)[i] = (*pfds)[*fd_count - 1];

    (*fd_count)--;

}

void process_client_event(int listener, struct pollfd **pfds, int *fd_size, int *fd_count, int i) {
    char buf[MAX_MSG_LEN];
    int sender_fd = (*pfds)[i].fd;
    int numbytes = recv(sender_fd, buf, sizeof(buf), 0);

    if (numbytes == -1) {
        perror("pollchat: client: recv");
        fprintf(stderr, "pollchat: Could not process message of client on socket %d\n", sender_fd);
        return;
    }
    if (numbytes == 0) {
        process_hangup(pfds, fd_size, fd_count, i);
        return;
    }

    buf[numbytes] = '\0';

    // log message to server
    printf("pollchat: Client on socket %d sent: %.*s", sender_fd, numbytes, buf);

    // loop through all fds, and send them this message.
    for (int j = 1; j < *fd_count; j++) {
        if (j != i)
            send((*pfds)[j].fd, buf, numbytes, 0);
    }
    
}

void process_polling_event(int listener, struct pollfd **pfds, int *fd_size, int *fd_count) {

    for (int i = 0; i < *fd_count; i++) {

        if ((*pfds)[i].revents & (POLLIN | POLLHUP)) {
            // either a polling event is POLLIN (input received) or POLLHUP (hangup)
            if ((*pfds)[i].fd == listener) {
                // the polling event generating FD is just the listener
                // thus, a client event has occured (create new client)
                create_new_client(listener, pfds, fd_size, fd_count);
            }

            else {
                // the polling event generating FD is one of the clients
                // thus, either one of the clients is sending a message, or they have hung up
                process_client_event(listener, pfds, fd_size, fd_count, i);
            }
        }

    }

}

int main(void) {

    int listener_sockfd = create_listener_socket();
    if (listener_sockfd == -1) {
        fprintf(stderr, "Error fetching socket for listener.\n");
        exit(1);
    }

    int fd_size = 5; // we can modify the size if we want.
    struct pollfd *pfds = malloc(sizeof(struct pollfd) * fd_size);

    pfds[0].fd = listener_sockfd;
    pfds[0].events = POLLIN;

    int fd_count = 1;

    // main events loop
    while (1) {

        int poll_count = poll(pfds, fd_size, -1); // poll with no timeout

        if (poll_count == -1) {
            perror("pollchat: poll");
            exit(1);
        }

        // process what has occured.
        process_polling_event(listener_sockfd, &pfds, &fd_size, &fd_count);
    }

}
