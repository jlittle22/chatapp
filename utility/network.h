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


class NetworkFormatter {
public:
	NetworkFormatter();
	~NetworkFormatter();
	void setOpCode(uint8_t oc);
	void setMessage(std::string msg);
    uint8_t getOpCode();
    std::string getMessage();

	void parseNetworkForm(std::string offTheWire);

	std::string networkForm();
private:
    uint8_t op_code;
    std::string msg;
};

std::string serialize_int(uint32_t x);

uint32_t deserialize_int(std::string x);

void print_addrinfo(struct addrinfo p);

void get_ip_addr_as_string(struct addrinfo p, char (*ipstr)[46]);

#endif