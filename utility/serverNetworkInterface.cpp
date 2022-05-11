#include "serverNetworkInterface.h"
#include "network.h"
#include <sys/select.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <unistd.h>

using namespace std;

ServerNetworkInterface::ServerNetworkInterface(int listener_fd, struct timeval tv) {
    subs_lock = PTHREAD_MUTEX_INITIALIZER;
    msgs_lock = PTHREAD_MUTEX_INITIALIZER;
    q_sig = PTHREAD_COND_INITIALIZER;
    listener = listener_fd;
    timeout = tv;
    this->addSubscriber(listener);
}


ServerNetworkInterface::~ServerNetworkInterface() {
    // do nothing
}

void ServerNetworkInterface::addSubscriber(int fd) {
    pthread_mutex_lock(&subs_lock);
    subscribers.insert(fd);
    pthread_mutex_unlock(&subs_lock);
}

void ServerNetworkInterface::removeSubscriber(int fd) {
    pthread_mutex_lock(&subs_lock);
    subscribers.erase(fd);
    pthread_mutex_unlock(&subs_lock);
}

void ServerNetworkInterface::broadcastMessage(string message, int fd_to_exclude) {
    
}

string ServerNetworkInterface::readNextMessage(int *fd_sender) {
    pthread_mutex_lock(&msgs_lock);
    while (messages.empty())
        pthread_cond_wait(&q_sig, &msgs_lock);
    pair<int, string> res = messages.front();
    messages.pop();
    pthread_mutex_unlock(&msgs_lock);
    *fd_sender = get<0>(res);
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
    subscribers.insert(new_conn_fd);
}

void ServerNetworkInterface::monitorSubscribers() {
    fd_set readfds;

    while (1) { 
        FD_ZERO(&readfds);
        int max_fds = 0;
        pthread_mutex_lock(&subs_lock);
        for (auto it = subscribers.begin(); it != subscribers.end(); it++) {
            if (*it > max_fds) max_fds = *it;
            FD_SET(*it, &readfds);
        }
        pthread_mutex_unlock(&subs_lock);    
        
        select(max_fds + 1, &readfds, NULL, NULL, NULL);

        pthread_mutex_lock(&subs_lock);
        for (auto it = subscribers.begin();  it != subscribers.end();) {
            if (FD_ISSET(*it, &readfds)) {
                if (*it == listener) {
                    this->acceptConnection();
                    it++;
                    continue;
                }
                // we have data -- just read its message and add it to the queue i
                char msg_size_buffer[sizeof(uint32_t)];
                int bytes_read = 0;
                while (bytes_read < sizeof(msg_size_buffer)) {
                    int res = recv(*it, msg_size_buffer + bytes_read, sizeof(msg_size_buffer) - bytes_read, 0);
                    if (res == 0) {
                        // client disconnected early
                        printf("[ServerNetworkInterface] Client %d disconnected.\n", *it);
                        close(*it);
                        it = subscribers.erase(it);
                        break;
                    } else if (res == -1) {
                        // error
                        perror("[ServerNetworkInterface] monitorSubscribers -- recv");
                        close(*it);
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
                    int res = recv(*it, bytes + bytes_read, msg_size - bytes_read, 0);
                    if (res == 0) {
                        printf("[ServerNetworkInterface] Client %d disconnected unexpectedly.\n", *it);
                        close(*it);
                        it = subscribers.erase(it);
                        break;
                    } else if (res == -1) {
                        perror("[ServerNetworkInterface] monitorSubscribers -- recv");
                        close(*it);
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
                pair<int, string> p;
                p.first = *it;
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


