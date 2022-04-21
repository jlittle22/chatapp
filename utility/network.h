#ifndef NETWORK_H
#define NETWORK_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

void print_addrinfo(struct addrinfo p);

void get_ip_addr_as_string(struct addrinfo p, char (*ipstr)[46]);

#endif