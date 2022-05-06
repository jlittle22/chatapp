#include "network.h"
#include <stdlib.h>
#include <stdio.h>
#include <sstream>

using namespace std;

#define STREAM_LEN_OFFSET 0
#define OP_CODE_OFFSET 4
#define MESSAGE_OFFSET 5

std::string serialize_int(uint32_t x) {
  uint32_t network_x = htonl(x);
  std::stringstream sstream;
  const char* ptr = reinterpret_cast<char*>(&network_x);
  sstream.write(ptr, sizeof(network_x));
  return sstream.str();
}

uint32_t deserialize_int(std::string x) {
  uint32_t new_item;
  std::stringstream ss(x);
  ss.read(reinterpret_cast<char*>(&new_item), sizeof(new_item));
  return ntohl(new_item);
}

NetworkFormatter::NetworkFormatter() {
    op_code = 0;  // default to sending a chat message
    msg = "";
}

void NetworkFormatter::parseNetworkForm(string offTheWire) {
    size_t str_len = offTheWire.length();
    uint32_t data_len = deserialize_int(offTheWire.substr(STREAM_LEN_OFFSET, sizeof(uint32_t)));
    if (str_len != data_len) {
        fprintf(stderr, "Warning: The network data claims to be of length %u, but it's actually length %lu.\n", data_len, str_len);
    }

    uint8_t op_code = uint8_t(offTheWire[OP_CODE_OFFSET]);

    msg = offTheWire.substr(MESSAGE_OFFSET);

    if (msg.length() != data_len - MESSAGE_OFFSET) {
        fprintf(stderr, "Warning: The network claims that the message is %u bytes long, but it's actually %lu.\n", data_len - MESSAGE_OFFSET, msg.length());
    }
}

uint8_t NetworkFormatter::getOpCode() {
    return op_code;
}

string NetworkFormatter::getMessage() {
    return msg;
}

NetworkFormatter::~NetworkFormatter() {
    // do nothing
}

void NetworkFormatter::setOpCode(uint8_t oc) {
    op_code = oc;
}

void NetworkFormatter::setMessage(string m) {
    msg = m;
}

string NetworkFormatter::networkForm() {
    size_t msg_len = msg.length();
    uint32_t stream_size = sizeof(uint32_t) + sizeof(uint8_t) + msg_len;
    string num_bytes_str = serialize_int(stream_size);
    char op_code_char = char(op_code);

    char bytes[stream_size];
    size_t i;
    for (i = 0; i < sizeof(uint32_t); i++) {
        bytes[i] = num_bytes_str[i];
    }

    bytes[i++] = op_code_char;
    
    size_t j;
    for (j = 0; j < msg_len; j++) {
        bytes[j + i] = msg[j];
    }

    if (j + i != stream_size) {
        fprintf(stderr, "Error: j + i == %lu but stream_size == %d\n", j+i, stream_size);
    }

    string forTheWire(bytes, stream_size);
    return forTheWire;
}

void get_ip_addr_as_string(struct addrinfo p, char (*ipstr)[46]) { 
    print_addrinfo(p);
    void *address;
    if (p.ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in*)p.ai_addr;
        address = &(ipv4->sin_addr);
    } else {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)p.ai_addr;
        address = &(ipv6->sin6_addr);
    }
    inet_ntop(p.ai_family, address, *ipstr, sizeof(*ipstr));
}

void print_addrinfo(struct addrinfo p) {
    void *address;
    const char *ipversion;
    char ipstr[INET6_ADDRSTRLEN];

    if (p.ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in*)p.ai_addr;

        address = &(ipv4->sin_addr);
        ipversion = "IPv4";
    } else {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)p.ai_addr;

        address = &(ipv6->sin6_addr);
        ipversion = "IPv6";
    }

    inet_ntop(p.ai_family, address, ipstr, sizeof ipstr);
    printf("  %s: %s\n", ipversion, ipstr);
    printf("  Family: %d\n", p.ai_family);
    printf("  Sock Type: %d\n", p.ai_socktype);
    printf("  Protocol: %d\n", p.ai_protocol);
}




