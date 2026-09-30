import socket

def send(cmd):
    s = socket.socket()
    s.connect(('localhost', 6380))
    s.send((cmd + '\r\n').encode())
    res = s.recv(1024).decode().strip()
    s.close()
    return res

print("[1] Filling cache capacity (5 keys)...")
for i in range(1, 6):
    send(f"set k{i} value{i}")
print("-> Added: k1, k2, k3, k4, k5 (Cache is now full)")

print("\n[2] Accessing k1 to promote it to Most Recently Used...")
res_k1 = send("get k1")
print(f"-> Looked up k1: {res_k1} (k1 is now MRU)")

print("\n[3] Inserting k6 (Capacity is 5, so least recently used will be evicted)...")
send("set k6 value6")
print("-> Added: k6")

print("\n[4] Verifying eviction status:")
print("-> Lookup k2 (was least recently used):", send("get k2"))
print("-> Lookup k1 (was promoted):", send("get k1"))
print("-> Lookup k6 (recently added):", send("get k6"))
