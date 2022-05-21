#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <network.h>
#include <serverNetworkInterface.h>
#include <unordered_set>
#include <sys/select.h>
#include <iostream>
#include <signal.h>

#define PORT "9999"
#define BACKLOG_SIZE 10

using namespace std;

int listen(const char* port);
void* monitor_thread(void*arg);

int main() {
    signal(SIGPIPE, SIG_IGN);
    
    int listener = listen(PORT);

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    ServerNetworkInterface sni = ServerNetworkInterface(listener, timeout);
    
    pthread_t t;
    if (pthread_create(&t, NULL, monitor_thread, &sni) != 0) {
        fprintf(stderr, "[Server] Fatal Error: failed to create monitoring thread.\n");
        perror("pthread_create");
        return 1;
    }

    NetworkFormatter f = NetworkFormatter();

    string msg = "";
    ServerNetworkInterface::SubscriberContext c;
    
    while (1) {
        msg = sni.readNextMessage(&c);
        f.parseNetworkForm(msg);
        printf("Sender: %d, Op code: %d, Msg: %s\n", c.fd, f.getOpcode(), f.getData().c_str());
        switch(f.getOpcode()) {
            case C2S_CHAT_SENT: {
                // need to read message, broadcast it to all clients except sender
                sni.broadcastMessage(f.getData(), c.fd);
                f.setOpcode(S2C_CHAT_ACK);
                f.setData("");
                sni.sendMessage(f.networkForm(), c.fd);
                break;
            }
            default:
                printf("Unknown operation: %d... ignoring.\n", f.getOpcode());
                f.setOpcode(S2C_ERROR);
                f.setData("");
                sni.sendMessage(f.networkForm(), c.fd);
        }
    }

    return 0;
}

void* monitor_thread(void* arg) {
    ServerNetworkInterface *sni_ptr = (ServerNetworkInterface *)arg;
    sni_ptr->monitorSubscribers();
    return NULL;
}

int listen(const char* port) {
    struct addrinfo hints;
    struct addrinfo *res;
    int yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, port, &hints, &res);

    if (status != 0) {
        fprintf(stderr, "[server] getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int listener;
    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener == -1) {
            perror("socket");
            continue;
        }

        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (::bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
            close(listener);
            perror("bind");
            continue;
        }

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

    printf("[server] socket fd %d listening at localhost:%s ...\n", listener, PORT);

    return listener;
}

