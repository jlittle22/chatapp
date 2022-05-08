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

using namespace std;

class ClientNetworkInterface {
public:
	ClientNetworkInterface(string);
	~ClientNetworkInterface();
	string readNext();
	bool areMessages();
	size_t sendMessage(string);
private:

};

#endif