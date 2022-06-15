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


enum FormType {
	CHAT_SEND_C2S = 0,
	CHAT_ACK_S2C,
	CHAT_SEND_S2C,
	ERROR_S2C
};

struct ChatSendC2S {
	std::string msg;
};

struct ChatSendS2C {
	std::string senderId;
	std::string msg;
};

struct ErrorS2C {
	std::string errMsg;
};

class NetworkFormatter {
	public:
		void setForm(FormType opcode, void* data);

		FormType getType();
		void* getData();
		
		std::string serialize();
		void deserialize(std::string form);
	private:
		FormType opcode;
		void* data;
};

std::string serialize_int(uint32_t x);
uint32_t deserialize_int(std::string x);
std::string sockaddr_to_ip_string(struct sockaddr *sa);
void print_addrinfo(struct addrinfo p);
void get_ip_addr_as_string(struct addrinfo p, char (*ipstr)[46]);

#endif
