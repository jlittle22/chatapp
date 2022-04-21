#include "network.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void get_ip_addr_as_string(struct addrinfo p, char (*ipstr)[46]) { 
    print_addrinfo(p);
    void *address;
    if (p.ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in*)p.ai_addr;
        address = &(ipv4->sin_addr);
    } else {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)p.ai_addr;
        address = &(ipv6->sin6_addr);
    }
    inet_ntop(p.ai_family, address, *ipstr, sizeof(*ipstr));
}

void print_addrinfo(struct addrinfo p) {
    void *address;
    const char *ipversion;
    char ipstr[INET6_ADDRSTRLEN];

    if (p.ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in*)p.ai_addr;

        address = &(ipv4->sin_addr);
        ipversion = "IPv4";
    } else {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)p.ai_addr;

        address = &(ipv6->sin6_addr);
        ipversion = "IPv6";
    }

    inet_ntop(p.ai_family, address, ipstr, sizeof ipstr);
    printf("  %s: %s\n", ipversion, ipstr);
    printf("  Family: %d\n", p.ai_family);
    printf("  Sock Type: %d\n", p.ai_socktype);
    printf("  Protocol: %d\n", p.ai_protocol);
}




