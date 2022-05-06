import socket

host = "chat.johnsnlittle.com" #socket.gethostname()
port = 9999                   # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print("trying ", host, ":", port, sep="")
s.connect((host, port))
print("connected!")
s.sendall(b'Hello, world')
# data = s.recv(1024)
s.close()
# print('Received', repr(data))