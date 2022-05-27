#include "clientNetworkInterface.h"
#include "network.h"
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "network.h"

using namespace std;


static int safeSend(int fd, const void* msg, int len, int flags) {
    int bytes_sent = 0;
    while (bytes_sent < len) {
        int res = send(fd, (char*)msg + bytes_sent, len - bytes_sent, flags);
        if (res == -1) {
            return -1;
        }
        bytes_sent += res;
    }

    return bytes_sent;
} 

ClientNetworkInterface::ClientNetworkInterface(string host) {
    addrinfo* searchResults = searchForHost(host);
    server_fd = connectToHost(searchResults);
}   

ClientNetworkInterface::~ClientNetworkInterface(){
    close(server_fd);
}

string ClientNetworkInterface::readNextMessage() {
    fd_set readfds;    
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    
    select(server_fd + 1, &readfds, NULL, NULL, NULL);
    
    char msg_size_buffer[sizeof(uint32_t)];
    int bytes_read = 0;
    while (bytes_read < sizeof(msg_size_buffer)) {
        int res = recv(server_fd, msg_size_buffer + bytes_read, sizeof(msg_size_buffer) - bytes_read, 0);
        if (res == 0) {
            printf("[ClientNetworkInterface] Server disconnected.\n");
            break;
        } else if (res == -1) {
            // @BRADEN: I think you need to add some checks for a "would block" error code
            // which would indicate that the server read timed out...
            perror("[ClientNetworkInterface] readNextMessage -- recv");
            break;
        } else {
            bytes_read += res;
        }
    }

    if (bytes_read != sizeof(msg_size_buffer)) {
        // this happens when the recv experiences an error or times out...
        exit(1); // maybe don't just crash? idk you can do whatever you think appropriate
    }


    string msg_size_str(msg_size_buffer, sizeof(msg_size_buffer));
    int msg_size = deserialize_int(msg_size_str);

    char bytes[msg_size];
    for (int i = 0; i < sizeof(msg_size_buffer); i++) {
        bytes[i] = msg_size_buffer[i];
    }

    while(bytes_read < msg_size) {
        int res = recv(server_fd, bytes + bytes_read, msg_size - bytes_read, 0);
        if (res == 0) {
            printf("[ClientNetworkInterface] Server disconnected unexpectedly.\n");
            break;
        } else if (res == -1) {
            // Same shit here
            perror("[ClientNetworkInterface] readNextMessage -- recv");
            break;
        } else {
            bytes_read += res;
        }
    }

    if (bytes_read != msg_size) {
        // do something here
    }

    string bytes_str(bytes, msg_size);

    return bytes_str;
}

bool ClientNetworkInterface::areMessages() {
    return false;
}

size_t ClientNetworkInterface::sendMessage(string message) {
    const char* msg = message.c_str();
    size_t msg_len = strlen(msg);
    int bytes_sent = safeSend(server_fd, msg, msg_len, 0);
    
    // @BRADEN: Add error handling here.
    
    return 0;
}

addrinfo* ClientNetworkInterface::searchForHost(string host) {
    // connect to host
    struct addrinfo hints, *res;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    if ((status = getaddrinfo(host.c_str(), PORT, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s \"%s\"\n", gai_strerror(status), host.c_str());
        exit(1);
    }

    if (res == NULL) {
        fprintf(stderr, "failed to connect to %s\n", host.c_str());
        exit(1);
    }   

    return res;
}

int ClientNetworkInterface::connectToHost(addrinfo* address) { 
    int sock;
    struct addrinfo *p;
    for (p = address; p != NULL; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1) {
            perror("socket");
            continue;
        }

        if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("connect");
            continue;
        }

        break;
    }

    freeaddrinfo(address);

    if (p == NULL) {
        fprintf(stderr, "[client] failed to connect to all addresses.\n");
        exit(1);
    }

    return sock;
}
