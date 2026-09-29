import socket
import time

def send_command(host, port, cmd):
    s = socket.socket()
    s.connect((host, port))
    s.send((cmd + '\r\n').encode())
    response = s.recv(1024).decode()
    s.close()
    return response.strip()

def benchmark(host, port, n=1000):
    # SET benchmark
    start = time.time()
    for i in range(n):
        send_command(host, port, f"set key{i} value{i}")
    set_time = time.time() - start

    # GET benchmark
    start = time.time()
    for i in range(n):
        send_command(host, port, f"get key{i}")
    get_time = time.time() - start

    print(f"  SET {n} keys: {set_time:.2f}s ({int(n/set_time)} req/s)")
    print(f"  GET {n} keys: {get_time:.2f}s ({int(n/get_time)} req/s)")

print("=== Your KV Store ===")
benchmark("localhost", 6379)

print("\n=== Redis ===")
benchmark("localhost", 6380)