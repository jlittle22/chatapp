CPP = g++
CPPFLAGS = -Wall -std=c++11 -L $(LIB) -I $(LIB)

LIB := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/utility/)

ARCHIVE = $(LIB)/libutility.a

all: server client

client: client/client.cpp $(ARCHIVE)
	$(CPP) $(CPPFLAGS) -o client/client client/client.cpp -l utility
server: server/server.cpp $(ARCHIVE)
	$(CPP) $(CPPFLAGS) -o server/server server/server.cpp -l utility

clean:
	rm -f server/server *.o 
	rm -f client/client *.o