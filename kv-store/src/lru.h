#pragma once
#include <list>
#include <unordered_map>
#include <string>

class LRUCache {
public:
    LRUCache(int capacity);
    std::string get(const std::string& key);
    void put(const std::string& key, const std::string& value);
    void remove(const std::string& key);
private:
    int capacity;
    std::list<std::pair<std::string, std::string>> cache_list;
    std::unordered_map<std::string, std::list<std::pair<std::string, std::string>>::iterator> cache_map;
};
