#include <functional>

#include "http/HttpServer.h"
#include "http/HttpData.h"
#include "middleware/StaticFile.h"

void testFunction(HttpData &data) {}

int main() {
    HttpServer server;
    StaticFile static_file("/home/siskon/Desktop/test", "/");
    server.Get(static_file.getRegex(), static_file.getCallback());
    server.run();
    return 0;
}