#include "persistence.h"
#include <fstream>
#include <sstream>
#include <iostream>

Persistence::Persistence(const std::string& filename) : filename(filename) {}

void Persistence::save(const std::string& key, const std::string& value) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << "set " << key << " " << value << "\n";
    }
}

void Persistence::remove(const std::string& key) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << "delete " << key << "\n";
    }
}

void Persistence::load(std::unordered_map<std::string, std::string>& store) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string cmd, key, value;
        stream >> cmd >> key;

        if (cmd == "set") {
            stream >> value;
            store[key] = value;
        } else if (cmd == "delete") {
            store.erase(key);
        }
    }
    std::cout << "Store loaded from disk\n";
}