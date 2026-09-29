#include "lru.h"
LRUCache::LRUCache(int capacity) : capacity(capacity) {}
std::string LRUCache::get(const std::string& key){
    auto it = cache_map.find(key);
    if (it == cache_map.end()) return "NULL";
    cache_list.splice(cache_list.begin(), cache_list, it->second);
    return it->second->second;
}

void LRUCache::put(const std::string& key, const std::string& value){
    auto it = cache_map.find(key);
    if (it != cache_map.end()) {
        it->second->second = value;
        cache_list.splice(cache_list.begin(), cache_list, it->second);
        return;
    }
    if ((int)cache_list.size() >= capacity) {
        auto last = cache_list.back();
        cache_map.erase(last.first);
        cache_list.pop_back();
    }
    cache_list.emplace_front(key, value);
    cache_map[key] = cache_list.begin();
}

void LRUCache::remove(const std::string& key) {
    auto it = cache_map.find(key);
    if (it != cache_map.end()) {
        cache_list.erase(it->second);
        cache_map.erase(it);
    }
}