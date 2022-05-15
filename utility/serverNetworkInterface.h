#ifndef SERVER_NETWORK_H
#define SERVER_NETWORK_H

#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <string>
#include <pthread.h>
#include <utility>


class ServerNetworkInterface {
    public:
        ServerNetworkInterface(int listener_fd, struct timeval timeout);
        ~ServerNetworkInterface();
        struct SubscriberContext {
            std::string ip_str;
            int fd;
        };
        void broadcastMessage(std::string message, int fd_to_exclude);
        void sendMessage(std::string message, int destination_fd);
        std::string readNextMessage(SubscriberContext *sender);
        void monitorSubscribers();

    private:
        typedef std::queue<std::pair<SubscriberContext, std::string>> MessagesQueue;
        typedef std::unordered_map<int, SubscriberContext*> SubscribersMap;
        int listener;
        struct timeval timeout;
        pthread_mutex_t subs_lock;
        pthread_mutex_t msgs_lock;
        pthread_cond_t q_sig;
        MessagesQueue messages;
        SubscribersMap subscribers;
        SubscribersMap::iterator eraseSubscriber(SubscribersMap::iterator it);
        void acceptConnection();
};

#endif
