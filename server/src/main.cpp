#include "http/HttpServer.h"
#include "middleware/StaticFile.h"

int main() {
    HttpServer server;
    StaticFile static_file("/home/siskon/Desktop/test", "/");
    server.Get(static_file.getRegex(), static_file.getCallback());
    server.run();
    return 0;
}