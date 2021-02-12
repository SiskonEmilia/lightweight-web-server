#include "http/HttpServer.h"

int main() {
    HttpServer server;
    server.run(8, 10240);
    return 0;
}