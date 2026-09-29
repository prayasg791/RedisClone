# KV Store

A Redis-like key-value store built from scratch in C++. This project implements a TCP server, a command parser, LRU eviction, and disk persistence — all without using any external libraries.

---

## What is a Key-Value Store?

A key-value store is one of the simplest types of databases. Instead of tables and rows like SQL, it just stores pairs:

```
key   →   value
"name"  →  "Drishti"
"age"   →  "20"
```

You store something with a key, and later retrieve it using that same key. Redis is one of the most popular key-value stores in the world, used by companies like Twitter, GitHub, and Snapchat. This project builds a simplified version of Redis from scratch.

---

## Features

- TCP server on port 6380
- GET / SET / DELETE commands
- LRU (Least Recently Used) eviction with a capacity of 5 keys
- Persistence to disk using an Append-Only File (AOF)
- Benchmarks vs Redis

---

## Project Structure

```
kv-store/
├── src/
│   ├── main.cpp          # Entry point, starts the server
│   ├── server.h          # Server class declaration
│   ├── server.cpp        # TCP server, command parsing, request handling
│   ├── lru.h             # LRUCache class declaration
│   ├── lru.cpp           # LRU eviction logic
│   ├── persistence.h     # Persistence class declaration
│   └── persistence.cpp   # Disk persistence using AOF
├── CMakeLists.txt
└── README.md
```

---

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
./Debug/kvserver.exe
```

---

## Usage

Once the server is running, you can interact with it using a simple Python client:

```python
import socket

while True:
    cmd = input("> ")
    if cmd == "exit":
        break
    s = socket.socket()
    s.connect(('localhost', 6380))
    s.send((cmd + '\r\n').encode())
    response = s.recv(1024).decode()
    print(response.strip())
    s.close()
```

### Supported Commands

| Command | Description | Example |
|---|---|---|
| `set key value` | Store a key-value pair | `set name Drishti` |
| `get key` | Retrieve a value by key | `get name` |
| `delete key` | Remove a key | `delete name` |

---

## Why Port 6380?

Redis uses port 6379 by default. Since this project was benchmarked alongside a real Redis instance, port 6380 was chosen to avoid conflict. This way both servers can run simultaneously on the same machine — Redis on 6379, this store on 6380.

---

## How the TCP Server Works

When you run the server, it:

1. Creates a **socket** — an endpoint for network communication
2. **Binds** it to port 6380 — attaches it to that port on your machine
3. **Listens** for incoming connections
4. In a loop, **accepts** a client and immediately spawns a dedicated thread to handle it, so the server can go back to accepting the next client without waiting

```
Client A                      Server                      Client B

|                             |                             |

|------- TCP connect -------->|                             |

|                             |<------- TCP connect --------|

|------- "set name A" ------->| (thread 1)                  |

|                             | (thread 2)<--- "set city B" |

|<------ "OK" ----------------|                             |

|                             |--------- "OK" -------------->|
```

This is called a **thread-per-connection** model. Each client gets its own dedicated thread for the lifetime of their request, so multiple clients can be served at the same time instead of one at a time. All access to the shared store and AOF file is protected by a `std::mutex`, so reads/writes to the data itself remain safe even though sockets are handled in parallel (see Limitation #2 below for the tradeoff this introduces).

---

## How LRU Eviction Works

### The Problem

Memory is not infinite. If you keep adding keys forever, the store will eventually run out of memory. You need a way to automatically remove old keys when the store gets too full.

### The Solution — LRU (Least Recently Used)

LRU eviction removes the key that hasn't been used in the longest time. The logic is: if you haven't used a key recently, you probably don't need it.

**Example with capacity = 5:**

```
set a 1   →  store: [a]
set b 2   →  store: [a, b]
set c 3   →  store: [a, b, c]
set d 4   →  store: [a, b, c, d]
set e 5   →  store: [a, b, c, d, e]  ← full!
get a     →  store: [b, c, d, e, a]  ← 'a' moved to front (recently used)
set f 6   →  store is full → evict least recently used → evict 'b'
             store: [c, d, e, a, f]
```

### How It's Implemented

LRU uses two data structures working together:

**1. A doubly linked list** — tracks order of use. Most recently used key is at the front, least recently used is at the back.

**2. A hash map** — maps each key directly to its node in the list for O(1) lookup.

```
List (front = most recent, back = least recent):
[f] <-> [a] <-> [e] <-> [d] <-> [c]

Map:
"a" → points to 'a' node in list
"f" → points to 'f' node in list
...
```

Every GET or SET moves the key to the front of the list. When eviction is needed, the back of the list is removed. Both operations are O(1) — instant, regardless of store size.

This capacity is currently set to 5 for demonstration. In a real system it would be set based on available memory.

---

## How Persistence Works

### The Problem

By default, data lives only in memory (RAM). If the server crashes or restarts, all data is lost.

### The Solution — Append-Only File (AOF)

Every time a SET or DELETE command is executed, it is appended to a file called `kv.aof`:

```
set a 1
set b 2
set c 3
delete b
set d 4
```

The file never gets edited — only new lines are added to the bottom. This is called an **append-only** file.

When the server restarts, it reads this file from top to bottom and **replays** every command to rebuild the store in memory:

```
replay line 1: set a 1    →  store: {a=1}
replay line 2: set b 2    →  store: {a=1, b=2}
replay line 3: set c 3    →  store: {a=1, b=2, c=3}
replay line 4: delete b   →  store: {a=1, c=3}
replay line 5: set d 4    →  store: {a=1, c=3, d=4}
```

This is the same persistence strategy used by Redis itself.

---

## Benchmarks vs Redis

Benchmarks were run using a Python script that sends 1000 SET and 1000 GET commands to both servers and measures time.

| Operation | This KV Store | Redis 7.0.15 |
|---|---|---|
| SET 1000 keys | 3.55s (281 req/s) | 3.19s (313 req/s) |
| GET 1000 keys | 3.55s (281 req/s) | 1.12s (892 req/s) |

### How the Benchmark Works

```python
for i in range(1000):
    send_command("set key{i} value{i}")   # measure total time
    
for i in range(1000):
    send_command("get key{i}")            # measure total time
```

Each command opens a new TCP connection, sends the command, reads the response, and closes. Total time divided by number of commands gives requests per second.

### What the Numbers Mean

- **SET performance is close** — only ~11% difference. Both stores write at similar speeds.
- **GET performance gap** — Redis is ~3x faster on GET because it has highly optimized internal data structures built and tuned over 15+ years.

These numbers should be taken in context — see the drawbacks section below for why the comparison isn't entirely fair.

---

## Drawbacks and Limitations

This is a learning project and a simplified model. Here is an honest breakdown of what it does not do:

### 1. One TCP Connection Per Request (No Connection Pooling)

Every single command opens a new TCP connection:

```
connect → send command → receive response → disconnect
connect → send command → receive response → disconnect
connect → send command → receive response → disconnect
```

Opening a TCP connection involves a **3-way handshake** (SYN, SYN-ACK, ACK) which takes time. Real Redis clients use **persistent connections** — they open one connection and send thousands of commands through it without reconnecting. This overhead is the biggest reason the benchmark numbers are lower than what the store is actually capable of.

### 2. Coarse-Grained Locking (No Per-Key Concurrency)

The server now uses a thread-per-connection model — each client is handled on its own thread, so multiple clients can connect and have their sockets processed in parallel. However, all access to the store and AOF file is protected by a single `std::mutex`, so only one command actually executes against the data at a time. Redis instead uses a single-threaded event loop with I/O multiplexing (select/epoll), avoiding locking overhead entirely by design. A further improvement here would be per-key locking or sharding the store across multiple mutexes to allow true concurrent writes to different keys.


### 3. LRU Capacity is Hardcoded

The LRU capacity is set to 5 keys for demonstration. In a real system, capacity would be configurable and based on available system memory.

### 4. AOF Grows Forever

The append-only file keeps growing. It never gets compacted. Redis solves this with **AOF rewriting** — periodically rewriting the file to remove redundant entries (e.g. if a key was set 100 times, only the last value matters). This store does not implement that.

### 5. No RESP Protocol

Real Redis uses RESP (Redis Serialization Protocol) for communication — a structured binary-safe format. This store uses plain text commands, which means it is not compatible with standard Redis clients like `redis-cli` or any Redis SDK.

### 6. No Error Recovery

If the server crashes mid-write, the AOF file could be left in a corrupt state. Redis handles this with checksums and partial write detection.

---

## What I Learned

- How TCP sockets work at a low level
- How to parse raw string commands into structured tokens
- Why LRU requires both a linked list and a hash map to achieve O(1) operations
- How append-only files provide crash-safe persistence
- The performance cost of per-connection overhead vs connection pooling
- How Redis achieves its speed and what makes it production-grade
- The tradeoffs between thread-per-connection concurrency and Redis's single-threaded event-loop model — and why coarse-grained locking is a reasonable first step but not the final answer for write-heavy concurrent workloads
---

## Tech Stack

- **Language:** C++ 17
- **Build system:** CMake
- **Platform:** Windows (Winsock2)
- **Benchmarking:** Python 3