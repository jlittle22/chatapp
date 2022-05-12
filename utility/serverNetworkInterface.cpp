#include "serverNetworkInterface.h"
#include "network.h"
#include <sys/select.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <unistd.h>

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

void ServerNetworkInterface::sendMessage(std::string message, int destination_fd) {
    if (safeSend(destination_fd, message.c_str(), message.length(), 0) == -1) {
        perror("[ServerNetworkInterface] sendMessage -- safeSend");
    }
}

ServerNetworkInterface::ServerNetworkInterface(int listener_fd, struct timeval tv) {
    subs_lock = PTHREAD_MUTEX_INITIALIZER;
    msgs_lock = PTHREAD_MUTEX_INITIALIZER;
    q_sig = PTHREAD_COND_INITIALIZER;
    listener = listener_fd;
    timeout = tv;


    SubscriberContext *c = new SubscriberContext();
    c->fd = listener;
    pthread_mutex_lock(&subs_lock);
    subscribers[listener] = c;
    pthread_mutex_unlock(&subs_lock);
}

ServerNetworkInterface::~ServerNetworkInterface() {
    close(listener);
}

void ServerNetworkInterface::broadcastMessage(string message, int fd_to_exclude) {
    pthread_mutex_lock(&subs_lock);
    for (auto it = subscribers.begin(); it != subscribers.end();) {
        if (it->first != fd_to_exclude) {
            if (safeSend(it->first, message.c_str(), message.length(), 0) == -1) {
                perror("[ServerNetworkInterface] broadcastMessage -- safeSend");
                delete it->second;
                it = subscribers.erase(it);
                continue;
            }
        }
        it++;
    }
    pthread_mutex_unlock(&subs_lock);   
}

string ServerNetworkInterface::readNextMessage(SubscriberContext *sender) {
    pthread_mutex_lock(&msgs_lock);
    while (messages.empty())
        pthread_cond_wait(&q_sig, &msgs_lock);
    pair<SubscriberContext, string> res = messages.front();
    messages.pop();
    pthread_mutex_unlock(&msgs_lock);
    sender->fd = get<0>(res).fd;
    sender->ip_str = get<0>(res).ip_str;
    return get<1>(res);
}

// NOT subscribers THREAD SAFE!!!
void ServerNetworkInterface::acceptConnection() {
    struct sockaddr connection;
    socklen_t addr_size = sizeof(connection);
    int new_conn_fd = accept(listener, (struct sockaddr *)&connection, &addr_size);
    if (new_conn_fd == -1) {
        printf("[ServerNetworkInterface] failed to accept a connection that we should have accepted.\n");
        return;
    }

    printf("[ServerNetworkInterface] adding a new connection (fd: %d).\n", new_conn_fd);
    if (setsockopt(new_conn_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) == -1) {
        perror("[ServerNetworkInterface] acceptConnection -- setsockopt");
        return;
    }
    printf("[ServerNetworkInterface] New connection is from IP address: %s\n", sockaddr_to_ip_string(&connection).c_str()); 
    SubscriberContext *c = new SubscriberContext();
    c->fd = new_conn_fd;
    c->ip_str = sockaddr_to_ip_string(&connection);
    subscribers[new_conn_fd] = c;
}

void ServerNetworkInterface::monitorSubscribers() {
    fd_set readfds;

    while (1) { 
        FD_ZERO(&readfds);
        int max_fds = 0;
        pthread_mutex_lock(&subs_lock);
        for (auto it = subscribers.begin(); it != subscribers.end(); it++) {
            if (it->first > max_fds) max_fds = it->first;
            FD_SET(it->first, &readfds);
        }
        pthread_mutex_unlock(&subs_lock);    
        
        select(max_fds + 1, &readfds, NULL, NULL, NULL);

        pthread_mutex_lock(&subs_lock);
        for (auto it = subscribers.begin();  it != subscribers.end();) {
            if (FD_ISSET(it->first, &readfds)) {
                if (it->first == listener) {
                    this->acceptConnection();
                    it++;
                    continue;
                }
                // we have data -- just read its message and add it to the queue i
                char msg_size_buffer[sizeof(uint32_t)];
                int bytes_read = 0;
                while (bytes_read < sizeof(msg_size_buffer)) {
                    int res = recv(it->first, msg_size_buffer + bytes_read, sizeof(msg_size_buffer) - bytes_read, 0);
                    if (res == 0) {
                        // client disconnected early
                        printf("[ServerNetworkInterface] Client %d disconnected.\n", it->first);
                        close(it->first);
                        delete it->second;
                        it = subscribers.erase(it);
                        break;
                    } else if (res == -1) {
                        // error
                        perror("[ServerNetworkInterface] monitorSubscribers -- recv");
                        close(it->first);
                        delete it->second;
                        it = subscribers.erase(it);
                        break;
                    } else {
                        bytes_read += res;
                    }
                }

                if (bytes_read != sizeof(msg_size_buffer)) {
                    continue;
                }

                string msg_size_str(msg_size_buffer, sizeof(msg_size_buffer));
                int msg_size = deserialize_int(msg_size_str);
     
                char bytes[msg_size];
                for (int i = 0; i < sizeof(msg_size_buffer); i++) {
                    bytes[i] = msg_size_buffer[i];
                }

                while(bytes_read < msg_size) {
                    int res = recv(it->first, bytes + bytes_read, msg_size - bytes_read, 0);
                    if (res == 0) {
                        printf("[ServerNetworkInterface] Client %d disconnected unexpectedly.\n", it->first);
                        close(it->first);
                        delete it->second;
                        it = subscribers.erase(it);
                        break;
                    } else if (res == -1) {
                        perror("[ServerNetworkInterface] monitorSubscribers -- recv");
                        close(it->first);
                        delete(it->second);
                        it = subscribers.erase(it);
                        break;
                    } else {
                        bytes_read += res;
                    }
                }

                if (bytes_read != msg_size) {
                    continue;
                }

                string bytes_str(bytes, msg_size);
                pair<SubscriberContext, string> p;
                p.first = *(it->second);
                p.second = bytes_str;

                pthread_mutex_lock(&msgs_lock);
                messages.push(p);
                pthread_cond_signal(&q_sig);
                pthread_mutex_unlock(&msgs_lock);

                it++;
            } else {
                // there is no data
                it++;
            }
        }
        pthread_mutex_unlock(&subs_lock);
    }
}


