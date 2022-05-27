#include "clientNetworkInterface.h"
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "network.h"

using namespace std;

int server_fd;
NetworkFormatter nformat;
ClientNetworkInterface::ClientNetworkInterface(string host) {
    nformat = NetworkFormatter();
    addrinfo* searchResults = searchForHost(host);
    server_fd = connectToHost(searchResults);
}   

ClientNetworkInterface::~ClientNetworkInterface(){
    close(server_fd);
}
string ClientNetworkInterface::readNext() {
    int size = deserialize_int(readString(server_fd, MSG_SIZE_FIELD_LENGTH));
    string res = readString(server_fd, size);
    return res;
}

string ClientNetworkInterface::readString(int fd, int len) {
    char buffer[len];
    readUntilFull(fd, buffer, len);
    return string(buffer, strlen(buffer));
}

void ClientNetworkInterface::readUntilFull(int fd, char* buffer, int target_len) {
    int bytes_read = 0;
    while (bytes_read < target_len) {
        int res = recv(fd, buffer + bytes_read, target_len - bytes_read, 0);
        if (res == 0) {
            fprintf(stderr, "[readUntilFull] server disconnected");
            exit(1);
        } else if (res == -1) {
            perror("[readUntilFull]");
            exit(1);
        } else {
            bytes_read += res;
        }
    }
}

bool ClientNetworkInterface::areMessages() {
    return false;
}
size_t ClientNetworkInterface::sendMessage(string message) {
    nformat.setData(message);
    nformat.setOpcode(0);
    string msg = nformat.networkForm();
    int len = msg.length();
    int bytes_sent = 0;
    while (bytes_sent < len) {
        int res = send(server_fd, msg.c_str() + bytes_sent, len - bytes_sent, 0);
        if (res == -1) {
            return -1;
        }
        bytes_sent += res;
    }
    return bytes_sent;
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