CPP = g++
CPPFLAGS = -Wall -std=c++11 -L $(LIB) -I $(LIB) -lpthread

LIB := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../utility/)

ARCHIVE = $(LIB)/libutility.a

all: client

client: client.cpp $(ARCHIVE)
	$(CPP) $(CPPFLAGS) -o client *.cpp -l utility -lncurses

clean:
	rm -f client *.o 
