#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <network.h>
#include <unordered_set>
#include <sys/select.h>

#define PORT "9999"
#define BACKLOG_SIZE 10

using namespace std;

int main() {

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

    printf("[server] socket fd %d listening at chat.johnsnlittle.com:%s ...\n", listener, PORT);

    unordered_set<int> subscribers;
    subscribers.insert(listener);

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    fd_set readfds;

    char msg_size_buffer[sizeof(uint32_t)];

    NetworkFormatter f = NetworkFormatter();

    while(1) {
        FD_ZERO(&readfds);
        int max_fd = 0;
        for (auto it = subscribers.begin(); it != subscribers.end(); it++) {
            FD_SET(*it, &readfds);
            if (*it > max_fd) {
                max_fd = *it;
            }
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);
        
        for (auto it = subscribers.begin(); it != subscribers.end();) {
            if (FD_ISSET(*it, &readfds)) {
                if (*it == listener) {
                    struct sockaddr connection;
                    socklen_t addr_size = sizeof(connection);
                    int new_connection_fd = accept(listener, (struct sockaddr *)&connection, &addr_size);
                    if (new_connection_fd == -1) {
                        printf("[server] failed to accept a connection.\n");
                        continue;
                    }
                    printf("[server] adding a connection (fd %d) to subscribers.\n", new_connection_fd);
                    subscribers.insert(new_connection_fd);
                    it++;
                } else {
                    printf("[server] fd %d sent data\n", *it);
                    int ret = recv(*it, msg_size_buffer, sizeof(msg_size_buffer), 0);
                    if (ret == 0) {
                        printf("[server] fd %d disconnected.\n", *it);
                        close(*it);
                        it = subscribers.erase(it);
                        continue;
                    } else if (ret == -1) {
                        perror("recv - incoming msg size");
                        close(*it);
                        it = subscribers.erase(it);
                        continue;
                    } else if (ret < (int)sizeof(msg_size_buffer)) {
                        fprintf(stderr, "[server] client %d sent a message that was too short... ignoring.\n", *it);
                        it++;
                        continue;
                    }
                    string msg_size(msg_size_buffer, sizeof(msg_size_buffer));
                    uint32_t data_len = deserialize_int(msg_size);
                    printf("[server] receiving %u bytes...\n", data_len);

                    char bytes[data_len];
                    uint32_t i;
                    for (i = 0; i < sizeof(msg_size_buffer); i++) {
                        bytes[i] = msg_size_buffer[i];
                    }

                    ret = recv(*it, bytes + sizeof(msg_size_buffer), data_len - sizeof(msg_size_buffer), 0);
                    if (ret == 0) {
                        printf("[server] Warning: client %d disconnected mid-message.\n", *it);
                        close(*it);
                        it = subscribers.erase(it);
                        continue;
                    } else if (ret == -1) {
                        perror("revc - incoming stream");
                        close(*it);
                        it = subscribers.erase(it);
                        continue;
                    } else if (ret < (int)(data_len - sizeof(msg_size_buffer))) {
                        printf("[server] Warning: client sent message that was shorter than expected. Got %lu when we expected %u...\n", ret + sizeof(msg_size_buffer), data_len);
                    }
                    fprintf(stderr, "read all of msg\n");
                    string offTheWire(bytes, ret + sizeof(msg_size_buffer));
                    f.parseNetworkForm(offTheWire);
                    printf("\nReceived Message:\nsize: %d\nop code: %u\nmsg: %s\n", (int)f.getMessage().length(), f.getOpCode(), f.getMessage().c_str());
                    it++;
                }
            } else {
                printf("[server] fd %d had nothing to read.\n", *it);
                it++;
            }
        }      
    }

    return 0;
}

