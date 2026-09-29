import socket
import threading
import time

def client(i):
    s = socket.socket()
    s.connect(('localhost', 6380))
    s.send(f"set key{i} value{i}\r\n".encode())
    response = s.recv(1024).decode()
    print(f"Client {i} got: {response.strip()}")
    s.close()

start = time.time()
threads = [threading.Thread(target=client, args=(i,)) for i in range(5)]
for t in threads:
    t.start()
for t in threads:
    t.join()
print(f"All 5 clients finished in {time.time() - start:.3f}s")