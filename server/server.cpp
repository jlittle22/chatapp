#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include "network.h"

#define PORT 9999

using namespace std;

int main() {

    get_address_info("www.google.com", NULL);

    return 0;
}