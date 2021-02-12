#pragma once

#include <memory>

#include "sockets/Socket.h"
#include "http/HttpData.h"

class HttpTimer;
class TimerManager;
class HttpServer;

class ConnectionSocket : public Socket {

    std::weak_ptr<HttpTimer> timer;
    HttpServer& server;

    // 禁止默认构造
    ConnectionSocket();

public:
    HttpData http_data;

    ConnectionSocket(HttpServer& server) : server(server) { };

    /**
     * @brief 如果当前 Connection 没有 Timer，则申请服务器为自身创建一个 Timer
    */
    bool setTimer(const time_t timeout);

    /**
     * @brief 移除当前 Connection 与 Timer 的绑定
    */
    void removeTimer();

    /**
     * @brief 处理超时事件：关闭文件符，通知 server 删除链接
    */
    void expireHandler();

    /**
     * @brief 析构时移除计时器，基类自动关闭 socket_fd
    */
    ~ConnectionSocket() {}
};