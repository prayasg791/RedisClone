#include "server.h"

int main() {
    Server server(6380);
    server.run();
    return 0;
}