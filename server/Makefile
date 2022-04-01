CPP = g++
CPPFLAGS = -Wall -L $(LIB) -l utility -I $(LIB)

LIB := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../utility/)

ARCHIVE = $(LIB)/libutility.a

all: server

server: server.cpp $(ARCHIVE)
	$(CPP) $(CPPFLAGS) -o server server.cpp $(EXTRACPP)

clean:
	 rm -f server *.o 
