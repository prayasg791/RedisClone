cmake_minimum_required(VERSION 3.16)
project(kv-store)
set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    src/main.cpp
    src/server.cpp
    src/lru.cpp
    src/persistence.cpp
)

add_executable(kvserver ${SOURCES})
target_link_libraries(kvserver ws2_32)