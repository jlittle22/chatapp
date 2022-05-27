import socket
import sys

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)

# Bind the socket to the address given on the command line
server_address = ('localhost', 9999)
sock.bind(server_address)

print('starting up on', sock.getsockname())
sock.listen(1)

while True:
    print('waiting for a connection')
    connection, client_address = sock.accept()
    try:
        print('client connected:', client_address)
        while True:
            res = input(">")
            res = str(len(res).to_bytes(4, "big"))  + res
            print("sending ", res)
            connection.sendall(bytearray(res.encode()))
    finally:
        connection.close()