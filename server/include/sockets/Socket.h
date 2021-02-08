#pragma once

#include <unistd.h>
#include <iostream>

/**
 * @brief RAII 风格的 Socket 管理基类
*/
class Socket {

    int socket_fd;

public:
    Socket() : socket_fd(-1) { }

    virtual ~Socket() {
        close();
    }

    virtual void close() {
        if (socket_fd > 0) {
            ::close(socket_fd);
            socket_fd = -1;
        }
    }

    const int getSocketFd() const {
        return socket_fd;
    }

    void setSocketFd(const int socket_fd) {
        if (this->socket_fd > 0) {
            std::cout << "Socket: This socket has been binds to one fd. Close it before trying to set it." << std::endl;
        } else if (socket_fd > 0) {
            this->socket_fd = socket_fd;
        }
    }
};