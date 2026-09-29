#pragma once
#include <string>
#include <unordered_map>

class Persistence {
public:
    Persistence(const std::string& filename);
    void save(const std::string& key, const std::string& value);
    void remove(const std::string& key);
    void load(std::unordered_map<std::string, std::string>& store);

private:
    std::string filename;
};