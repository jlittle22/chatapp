#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <inttypes.h>
#include <string>
#include <string.h>
#include "network.h"

#define PORT "9999"
#define TIMEOUT 1
#define MSG_BUFFER_SIZE 256

using namespace std;

class ClientNetworkInterface {
public:
	ClientNetworkInterface(std::string);
	~ClientNetworkInterface();
    std::string readNextMessage();
	bool areMessages();
	size_t sendMessage(std::string);
private:
    int server_fd;
	NetworkFormatter nformat;
    addrinfo* searchForHost(std::string);
    int connectToHost(addrinfo*);
	string readString(int, int);
	void readUntilFull(int, char*, int);
};

#endif
