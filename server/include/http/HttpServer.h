#pragma once

#include <vector>
#include <regex>

#include "sockets/ListenSocket.h"
#include "thread/ThreadPool.h"
#include "epoll/EpollManager.h"
#include "timer/TimerManager.h"
#include "http/HttpData.h"

class HttpTimer;
class ConnectionSocket;

class HttpServer {
public:
    typedef std::function<void(HttpData&)> RequestCallback;
    typedef struct ServeRule {
        // 检查 path 是否合法的正则表达式【废弃】
        // const static std::regex path_checker;
        std::regex path;
        HttpServer::RequestCallback callback;
        ServeRule() = delete;
        ServeRule(const std::regex &path, const HttpServer::RequestCallback &callback)
            : path(path), callback(callback) { }
    } ServeRule;

private:
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
    // GET 方法的匹配规则表
    std::vector<ServeRule> get_rules;
    
    /**
     * @brief 默认的回调函数，发送 404 响应
    */
    static void defaultRequestCallback(HttpData &);
    /**
     * @brief 处理发送循环，发送到不能发为止
     * @return 剩余需要发送的数据量
    */
    static int sendLoop(const int socket_fd, const char* buffer, int size_to_send);

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

    void handleSending(std::shared_ptr<ConnectionSocket> connection);

    std::shared_ptr<HttpTimer>
    createTimer(int timeout, int socket_fd);

    int getListenSocketFd() {
        return listen_socket.getSocketFd();
    }

    /**
     * @brief 注册 GET 规则
    */
    void Get(const std::regex &path, const RequestCallback& callback);

    /**
     * @brief 开始运行服务器
    */
    void run(int thread_num = 8, int max_queue_size = 16384, int timeout = 3000);
};