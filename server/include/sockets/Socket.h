#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
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

    /**
     * @brief 设置参数 fd 为非阻塞状态（防止 ET 最后读完的时候阻塞）
    */
    void setNonBlocking() {
        if (socket_fd <= 0) {
            std::cout << "Socket: Failed to set non-blocking, invalid socket_fd." << std::endl;
            return;
        }
        int new_opt = fcntl(socket_fd, F_GETFL) | O_NONBLOCK;
        fcntl(socket_fd, F_SETFL, new_opt);
    }

    /**
     * @brief 设置地址复用，方便重启
    */
    void setReuseable() {
        if (socket_fd <= 0) {
            std::cout << "Socket: Failed to set reuseable, invalid socket_fd." << std::endl;
            return;
        }
        int opt = 1;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
    }
};