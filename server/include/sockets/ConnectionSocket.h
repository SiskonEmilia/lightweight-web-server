#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <memory>

#include "sockets/Socket.h"
#include "timer/HttpTimer.h"
#include "timer/TimerManager.h"

class ConnectionSocket : public Socket {

    std::weak_ptr<HttpTimer> timer;
    socklen_t mLen;
    sockaddr_in mAddr;
    int fd;

public:
    ConnectionSocket() : fd(-1) { };

    void setFd(const int fd) {
        this->fd = fd;
    }

    const int getFd() {
        return fd;
    }

    /**
     * @brief 如果当前 Connection 没有 Timer，则根据给定参数创建并设置 Timer，返回 true。否则返回 false。
    */
    bool setTimer(const time_t timeout, TimerManager &time_manager);

    /**
     * @brief 移除当前 Connection 与 Timer 的绑定
    */
    void removeTimer();

    /**
     * @brief 处理超时事件
    */
    void expireHandler();

    ~ConnectionSocket();
};