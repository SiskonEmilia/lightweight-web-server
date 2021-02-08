#pragma once

#include "sockets/Socket.h"
#include "thread/ThreadPool.h"
#include "epoll/EpollManager.h"

class ConnectionSocket;

class HttpServer {
    // 计时器管理器
    static TimerManager timer_manager;
    // 当前存在的链接映射
    static std::unordered_map<int, std::shared_ptr<ConnectionSocket>> connection_map;

public:
    void handleConnection();

    void removeConnection(const int socket_fd);

    int getListenSocketFd();
    
};