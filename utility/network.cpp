#include "network.h"
#include <stdlib.h>
#include <stdio.h>
#include <sstream>

using namespace std;

#define FORM_LEN_OFFSET 0
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

void NetworkFormatter::setForm(FormType opcode, void* data) {
    this->opcode = opcode;
    this->data = data;
}

FormType NetworkFormatter::getType() {
    return opcode;
}

void* NetworkFormatter::getData() {
    return data;
}

std::string NetworkFormatter::serialize() {
    std::string data_str;
    size_t data_len;

    switch(opcode) {
        case CHAT_SEND_C2S: {
            data_str = ((ChatSendC2S*)data)->msg;
            data_len = data_str.length();
            break;
        } case CHAT_ACK_S2C: {
            data_str = "";
            data_len = 0;
            break;
        } default: {
            fprintf(stderr, "Error serializing network form: unknown opcode %u.\n", opcode);
            break;
        }
    }

    size_t form_len = sizeof(uint32_t) + sizeof(uint8_t) + data_len;
    std::string form_len_str = serialize_int(form_len);
    char opcode_char = char(opcode);

    char form[form_len];
    size_t i;
    for (i = 0; i < sizeof(uint32_t); ++i) {
        form[i] = form_len_str[i];
    }

    form[i++] = opcode_char;
    
    for (size_t j = 0; j < data_len; j++) {
        form[i + j] = data_str[j];
    }

    return std::string(form, form_len);
}

void NetworkFormatter::deserialize(std::string form) {
    uint32_t real_form_len = form.length();
    uint32_t form_len = deserialize_int(form.substr(FORM_LEN_OFFSET, sizeof(uint32_t)));

    if(form_len != real_form_len) {
        fprintf(stderr, "Error parsing network form: the network data claims to be of length %u, but it's actually length %u.\n", form_len, real_form_len);
    }

    opcode = (FormType)form[OPCODE_OFFSET];
    std::string data_str = form.substr(DATA_OFFSET);
    switch(opcode) {
        case CHAT_SEND_C2S: {
            data = &data_str;
            break;
        } default: {
            fprintf(stderr, "Error parsing network form: unknown opcode %u.\n", opcode);
            break;
        }
    }
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