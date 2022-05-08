import socket

# IF YOU WANNA USE THIS SCRIPT, JUST CHANGE THE msg BELOW
# AND RUN IT!

msg = "Hello, this is a new chat. how are you?"

#######################################

host = "localhost"
port = 9999
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))
msg_encoded = bytes(msg, 'UTF-8')
op_code = 0
op_code_encoded = op_code.to_bytes(1, 'big')
length = len(msg) + 1 + 4
length_encoded = length.to_bytes(4, 'big')
send_data = length_encoded + op_code_encoded + msg_encoded
print("sending ", len(send_data), " bytes to ", host, ":", port, sep="")
s.sendall(send_data)

# data = s.recv(1024)
s.close()
# print('Received', repr(data))