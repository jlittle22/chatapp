#include "network.h"
#include <stdlib.h>
#include <stdio.h>
#include <sstream>

using namespace std;

#define FORM_LEN_OFFSET 0
#define OPCODE_OFFSET 4
#define DATA_OFFSET 5

NetworkForm::NetworkForm(FormType opcode, std::string* data) {
    this->opcode = opcode;
    this->data = data;
    data_on_heap = false;
}

NetworkForm::NetworkForm(std::string form) {
    uint32_t real_form_len = form.length();
    uint32_t form_len = deserialize_int(form.substr(FORM_LEN_OFFSET, sizeof(uint32_t)));

    if(form_len != real_form_len) {
        fprintf(stderr, "Error deserializing network form: the network data claims to be of length %u, but it's actually length %u.\n", form_len, real_form_len);
    }
    
    opcode = (FormType)form[OPCODE_OFFSET];

    size_t n_fields = NUM_FIELDS[opcode];
    switch(n_fields) {
        case 0: {
            data = nullptr;
            data_on_heap = false;
            break;
        } case 1: {
            data = (std::string*)malloc(sizeof(std::string));
            data_on_heap = true;
            *data = form.substr(DATA_OFFSET);
            break;
        } default: {
            data = (std::string*)malloc(n_fields * sizeof(std::string));
            data_on_heap = true;

            size_t field_offset = DATA_OFFSET;
            for(size_t i = 0; i < n_fields; ++i) {
                field_offset += read_field(form.substr(field_offset), &data[i]);
            }
            break;
        }
    }
}

NetworkForm::~NetworkForm() {
    if(data_on_heap) {
        free(data);
    }
}

FormType NetworkForm::getType() {
    return opcode;
}

std::string* NetworkForm::getData() {
    return data;
}

std::string NetworkForm::serialize() {
    std::string data_str;

    size_t n_fields = NUM_FIELDS[opcode];
    switch(n_fields) {
        case 0: {
            data_str = "";
            break;
        } case 1: {
            data_str = data[0];
            break;
        } default: {
            for(size_t i = 0; i < n_fields; ++i) {
                data_str.append(serialize_int(data[i].length()));
                data_str.append(data[i]);
            }
            break;
        }
    }

    size_t form_len = sizeof(uint32_t) + sizeof(uint8_t) + data_str.length();

    std::string form = serialize_int(form_len);
    form.push_back(char(opcode));
    form.append(data_str);

    return form;
}

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

size_t read_field(std::string field, std::string* txt) {
    uint32_t len = deserialize_int(field.substr(0, sizeof(uint32_t)));
    *txt = field.substr(sizeof(uint32_t), len);

    return len;
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