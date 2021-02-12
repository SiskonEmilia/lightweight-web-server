#pragma once

#include "sockets/ListenSocket.h"
#include "thread/ThreadPool.h"
#include "epoll/EpollManager.h"
#include "timer/TimerManager.h"

class HttpTimer;
class ConnectionSocket;

class HttpServer {

    enum {
        Default_Buffer_Size = 4096
    };
    int buffer_size;
    // 服务器持有的监听 Socket
    ListenSocket listen_socket;
    // 计时器管理器
    TimerManager timer_manager;
    // Epoll 封装类
    std::shared_ptr<EpollManager> epoll_manager;
    // 当前存在的链接映射
    std::unordered_map<int, std::shared_ptr<ConnectionSocket>> connection_map;

public:
    /**
     * @brief 创建并初始化 server 类
    */
    HttpServer(int port = 8080, const char *ip = nullptr, int buffer_size = Default_Buffer_Size)
        : buffer_size(buffer_size), listen_socket(port, ip), timer_manager(), 
          epoll_manager(EpollManager::getInstance()), connection_map() {
        if (buffer_size <= 0) this->buffer_size = Default_Buffer_Size;
        epoll_manager->init(1024);
    }

    /**
     * @brief 通知监听 socket 处理连接请求，为连接创建 socket 管理类
    */
    void handleConnection(int timeout);

    /**
     * @brief 解除 connection 与 Timer 的绑定、删除 connection 映射，删除其对应的文件描述符
    */
    void removeConnection(const int socket_fd);

    void handleRequest(std::shared_ptr<ConnectionSocket> connection);

    std::shared_ptr<HttpTimer>
    createTimer(int timeout, int socket_fd);

    int getListenSocketFd() {
        return listen_socket.getSocketFd();
    }

    void run(int thread_num, int max_queue_size, int timeout = 3000);
};