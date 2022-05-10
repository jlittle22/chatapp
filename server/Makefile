CPP = g++
CPPFLAGS = -Wall -std=c++11 -L $(LIB) -I $(LIB)

LIB := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../utility/)

ARCHIVE = $(LIB)/libutility.a

all: server

server: server.cpp $(ARCHIVE)
	$(CPP) $(CPPFLAGS) -o server server.cpp -l utility

clean:
	rm -f server *.o 
