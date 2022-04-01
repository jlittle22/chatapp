#include "network.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void get_address_info(const char *addr, const char *service) {
	int status;
	struct addrinfo hints;
	struct addrinfo *results;

	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_UNSPEC;  // don't care about IPv4 or IPv6 distinction
	hints.ai_socktype = SOCK_STREAM;  // TCP stream
	hints.ai_flags = AI_PASSIVE;  // tell me what my IP is for this box

	if ((status = getaddrinfo(addr, service, &hints, &results)) != 0) {
		fprintf(stderr, "getaddrinfo error %s\n", gai_strerror(status));
		exit(status);
	}

    struct addrinfo *p = results;

    char ipstr[INET6_ADDRSTRLEN];


	for (; p != NULL; p = p->ai_next) {
        void *address;
        char *ipversion;

        if (p->ai_family == AF_INET) {
        	struct sockaddr_in *ipv4 = (struct sockaddr_in*)p->ai_addr;

        	address = &(ipv4->sin_addr);
        	ipversion = "IPv4";
        } else {
        	struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)p->ai_addr;

        	address = &(ipv6->sin6_addr);
        	ipversion = "IPv6";
        }

        inet_ntop(p->ai_family, address, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipversion, ipstr);

	}




	freeaddrinfo(results);

}