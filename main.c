#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

int main(int argc, char *argv[]) {

    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: showip hostname\n");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;


    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: &s\n", gai_strerror(status));
        exit(1);
    }

    printf("IP addresses for %s:\n\n", argv[1]);

    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        struct sockaddr_in *ipv4;

        if (p->ai_family == AF_INET) {
            ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);

            int s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

            if (s == -1) continue;

        }
        // inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        // printf("  %s: %s\n", ipver, ipstr);

    }

    freeaddrinfo(res);

    return 0;
}
