#include "network.h"
#include <stdlib.h>
#include <stdio.h>
#include <sstream>

using namespace std;

#define STREAM_LEN_OFFSET 0
#define OPCODE_OFFSET 4
#define DATA_OFFSET 5

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
    opcode = 0;  // default to sending a chat message
    data = "";
}

void NetworkFormatter::parseNetworkForm(string offTheWire) {
    size_t str_len = offTheWire.length();
    uint32_t data_len = deserialize_int(offTheWire.substr(STREAM_LEN_OFFSET, sizeof(uint32_t)));
    
    if (str_len != data_len) {
        fprintf(stderr, "Warning: The network data claims to be of length %u, but it's actually length %lu.\n", data_len, str_len);
    }

    opcode = uint8_t(offTheWire[OPCODE_OFFSET]);

    switch(opcode) {
        case C2S_CHAT_SENT: {
            data = offTheWire.substr(DATA_OFFSET);
            
            if (data.length() != data_len - DATA_OFFSET) {
                fprintf(stderr, "Warning in opcode %d: The network claims that the message is %u bytes long, but it's actually %lu.\n", opcode, data_len - DATA_OFFSET, data.length());
            }

            break;
        } case S2C_CHAT_ACK: {
            break;
        } default: {
            fprintf(stderr, "Warning: Unknown opcode %u.\n", opcode);
        }
    }    
}

uint8_t NetworkFormatter::getOpcode() {
    return opcode;
}

string NetworkFormatter::getData() {
    return data;
}

NetworkFormatter::~NetworkFormatter() {
    // do nothing
}

void NetworkFormatter::setOpcode(uint8_t oc) {
    opcode = oc;
}

void NetworkFormatter::setData(string m) {
    data = m;
}

string NetworkFormatter::networkForm() {
    size_t data_len = data.length();
    uint32_t stream_size = sizeof(uint32_t) + sizeof(uint8_t) + data_len;
    string num_bytes_str = serialize_int(stream_size);
    char opcode_char = char(opcode);

    char bytes[stream_size];
    size_t i;
    for (i = 0; i < sizeof(uint32_t); i++) {
        bytes[i] = num_bytes_str[i];
    }

    bytes[i++] = opcode_char;

    switch(opcode) {
        case C2S_CHAT_SENT: {
            size_t j;
            for (j = 0; j < data_len; j++) {
                bytes[i + j] = data[j];
            }

            if (j + i != stream_size) {
                fprintf(stderr, "Error: j + i == %lu but stream_size == %d\n", j+i, stream_size);
            }
            break;
        }
        case S2C_CHAT_ACK: {
            break;
        }
    }

    string forTheWire(bytes, stream_size);
    return forTheWire;
}

string sockaddr_to_ip_string(struct sockaddr * sa) {
    char ipstr[INET6_ADDRSTRLEN];
    if (sa->sa_family == AF_INET) {
        // handle IPv4
        if (inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), ipstr, INET_ADDRSTRLEN) == NULL) {
            perror("[Network] sockaddr_to_ip_string");
            return "ERROR";
        }
    } else {
        // treat as IPv6
        if (inet_ntop(sa->sa_family, &(((struct sockaddr_in6 *)sa)->sin6_addr), ipstr, INET6_ADDRSTRLEN) == NULL) {
            perror("[Network] sockaddr_to_ip_string");
            return "ERROR";
        }
    }

    string res(ipstr, INET6_ADDRSTRLEN);
    return res;
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




