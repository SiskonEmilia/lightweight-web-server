#include <csignal>
#include <iostream>

#include "http/HttpServer.h"
#include "middleware/StaticFile.h"

// void signalHandler(int signal) {
//     std::cout << "Stopping server..." << std::endl;
//     exit(0);
// }

int main() {
    // signal(SIGINT, signalHandler);

    HttpServer server(8080, nullptr, 1024);
    StaticFile static_file("/home/siskon/Desktop/test", "/");
    server.Get(static_file.getRegex(), static_file.getCallback());
    server.run(4);
    return 0;
}