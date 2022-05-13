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
            data = connection.recv(4000)
            print('received "%s"' % data)
            if data:
                data = str(data.decode()) + " < send back from server"
                connection.sendall(bytearray(data.encode()))
                print('sent "%s"' % data)
            else:
                break
    finally:
        connection.close()