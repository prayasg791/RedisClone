import socket

while True:
    cmd = input("> ")
    if cmd == "exit":
        break
    s = socket.socket()
    s.connect(('localhost', 6379))
    s.send((cmd + '\r\n').encode())
    response = s.recv(1024).decode()
    print(response.strip())
    s.close()