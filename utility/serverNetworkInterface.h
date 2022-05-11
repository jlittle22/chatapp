#ifndef SERVER_NETWORK_H
#define SERVER_NETWORK_H

#include <unordered_set>
#include <queue>
#include <string>
#include <pthread.h>
#include <utility>

class ServerNetworkInterface {
    public:
        ServerNetworkInterface(int listener_fd, struct timeval timeout);
        ~ServerNetworkInterface();
        void addSubscriber(int fd);
        void removeSubscriber(int fd);
        void broadcastMessage(std::string message, int fd_to_exclude);
        std::string readNextMessage(int *fd_sender);  // blocks if there are no messages to read 
        void monitorSubscribers();

    private:
        int listener;
        struct timeval timeout;
        pthread_mutex_t subs_lock;
        pthread_mutex_t msgs_lock;
        pthread_cond_t q_sig;
        pthread_t t;
        std::queue<std::pair<int, std::string>> messages;
        std::unordered_set<int> subscribers;
        void acceptConnection();
};

#endif
