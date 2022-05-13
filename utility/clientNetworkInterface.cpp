#include "clientNetworkInterface.h"
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

int server_fd;

ClientNetworkInterface::ClientNetworkInterface(string host) {
    addrinfo* searchResults = searchForHost(host);
    server_fd = connectToHost(searchResults);
}   

ClientNetworkInterface::~ClientNetworkInterface(){
    close(server_fd);
}
string ClientNetworkInterface::readNext() {
    char msg_buffer[MSG_BUFFER_SIZE];
    int bytes_read = recv(server_fd, msg_buffer, sizeof(msg_buffer), 0);
    return string(msg_buffer, sizeof(msg_buffer));
}
bool ClientNetworkInterface::areMessages() {
    return false;
}
size_t ClientNetworkInterface::sendMessage(string message) {
    const char* msg = message.c_str();
    size_t msg_len = strlen(msg);
    int bytes_sent = send(server_fd, msg, msg_len, 0);
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
    int socket_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    if (socket_fd == -1){
        perror("sockect");
        exit(1);
    }

    if(connect(socket_fd, address->ai_addr, address->ai_addrlen) != 0) {
        perror("connect");
        exit(1);
    }
    

    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    // if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof(tv)) != 0) {
    //     perror("setsockopt");
    //     exit(1);
    // }

    
    return socket_fd;
}