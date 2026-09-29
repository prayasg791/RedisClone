#pragma once
#include <string>
#include <mutex>
#include <vector>
#include <mutex>
#include <winsock2.h>
#include "lru.h"
#include "persistence.h"

class Server {
public:
    Server(int port);
    void run();
private:
    int port;
    LRUCache store;
    Persistence persistence{"kv.aof"};
    std::mutex store_mutex;

    std::vector<std::string> parse_command(const std::string& command);
    std::string handle_command(const std::vector<std::string>& tokens);
    void handle_client(SOCKET client_fd);
};