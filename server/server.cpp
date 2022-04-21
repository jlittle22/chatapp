#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <network.h>

#define PORT "9999"
#define BACKLOG_SIZE 10

using namespace std;

int main() {

    char ipstr[INET6_ADDRSTRLEN];

    struct addrinfo hints;
    struct addrinfo *res;
    int yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, PORT, &hints, &res);

    if (status != 0) {
    	fprintf(stderr, "[server] getaddrinfo error: %s\n", gai_strerror(status));
    	exit(1);
    }

    int listener;
    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {
    	print_addrinfo(*p);

    	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener == -1) {
        	perror("socket");
        	continue;
        }

        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        	perror("setsockopt");
        	exit(1);
        } 

        if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
        	close(listener);
        	perror("bind");
        	continue;
        }
        
        print_addrinfo(*p);

        get_ip_addr_as_string(*p, &ipstr);

        break;
    }

    freeaddrinfo(res);

    if (p == NULL) {
    	fprintf(stderr, "[server] failed to bind to all ip addresses.\n");
    	exit(1);
    }

    if (listen(listener, BACKLOG_SIZE) == -1) {
    	perror("listen");
    	exit(1);
    }

    printf("[server] listening at %s ...\n", ipstr);







    return 0;
}