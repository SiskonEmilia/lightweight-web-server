#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <memory>

#include "sockets/Socket.h"


class ConnectionSocket;

/**
 * 
*/
class ListenSocket : public Socket {

    sockaddr_in served_addr;
    int epoll_fd;
    int port;
    const char *ip;

    /**
     * @brief 封装用于绑定 listen socket 和 IP\:port 的行为
    */
    void bind();

    /**
     * @brief 封装用于启动 listen socket 监听的行为
    */
    void listen();

public:
    /**
     * @brief 按给定的 IP 地址和 端口号 初始化一个 listen socket
    */
    ListenSocket(int port = 8080, const char *ip = nullptr);

    /**
     * @brief 若 listen socket 尚未关闭，则负责关闭 listen socket
    */
    ~ListenSocket();

    /**
     * @brief 关闭当前对象维护的 listen socket
    */
    void close();

    /**
     * @brief 尝试接受一个客户端发起的链接请求，若成功则为 connection socket 引用设置相关域
    */
    int accept(ConnectionSocket &) const;
};