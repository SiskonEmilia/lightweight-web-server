#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

int main() {
    const int port = 10024;
    const char *ip = nullptr;
    char buffer[4096];

    sockaddr_in mAddr;

    bzero(&mAddr, sizeof(mAddr));

    mAddr.sin_family = AF_INET;
    mAddr.sin_port   = htons(port);
    if (ip == nullptr) {
        inet_pton(AF_INET, ip, &mAddr.sin_addr);
    } else {
        mAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        cerr << "creat socket error in file <" << __FILE__ << "> "<< "at " << __LINE__ << endl;
        exit(0);
    }

    if (bind(socket_fd, (struct sockaddr*)&mAddr, sizeof(mAddr)) == -1) {
        cerr << "binding socket error in file <" << __FILE__ << "> "<< "at " << __LINE__ << endl;
        exit(0);
    }

    if (listen(socket_fd, 1024) == -1) {
        cerr << "listening socket error in file <" << __FILE__ << "> "<< "at " << __LINE__ << endl;
        exit(0);
    }

    cout << "Waiting for client's request" << endl;

    for (int connect_fd = -1; ; ) {
        if ((connect_fd = accept(socket_fd, nullptr, nullptr)) == -1) {
        cerr << "listening socket error in file <" << __FILE__ << "> "<< "at " << __LINE__ << endl;
            continue;
        }
        int recv_size = recv(connect, buffer, 4096, )
    }

    return 0;
}