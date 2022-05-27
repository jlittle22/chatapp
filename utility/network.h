#ifndef NETWORK_H
#define NETWORK_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <inttypes.h>
#include <string>

//////
// OPCODES
//////

enum Opcodes {
	C2S_CHAT_SENT = 0,
	S2C_CHAT_ACK,
	S2C_CHAT_SENT,
	S2C_ERROR
};
class NetworkFormatter {
public:
	NetworkFormatter();
	~NetworkFormatter();
	void setOpcode(uint8_t oc);
	void setData(std::string msg);
    uint8_t getOpcode();
    std::string getData();

	void parseNetworkForm(std::string offTheWire);

	std::string networkForm();
private:
    uint8_t opcode;
    std::string data;

	
};

std::string serialize_int(uint32_t x);

uint32_t deserialize_int(std::string x);

std::string sockaddr_to_ip_string(struct sockaddr *sa);

void print_addrinfo(struct addrinfo p);

void get_ip_addr_as_string(struct addrinfo p, char (*ipstr)[46]);

#endif
