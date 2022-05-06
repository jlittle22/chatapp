import socket

host = "chat.johnsnlittle.com" #socket.gethostname()
port = 9999                   # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))
msg = "Hello, this is a new chat. how are you?"
msg_encoded = bytes(msg, 'UTF-8')
op_code = 123
op_code_encoded = op_code.to_bytes(1, 'big')
length = len(msg) + 1 + 4
length_encoded = length.to_bytes(4, 'big')
send_data = length_encoded + op_code_encoded + msg_encoded
print("seding len: ", len(send_data))
s.sendall(send_data)

# data = s.recv(1024)
s.close()
# print('Received', repr(data))