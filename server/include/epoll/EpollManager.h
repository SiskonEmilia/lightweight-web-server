#pragma once

#include <sys/epoll.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "timer/TimerManager.h"
#include "utils/Uncopyable.h"

// 如果在 Linux 2.6.9 以下版本，使用以下宏定义以避免 epoll 不接受 nullptr 作为 event
// #define NULL_EVENT_NOT_ALLOWED

class HttpServer;

/**
 * @brief Epoll 行为管理类，单例模式
*/
class EpollManager : private Uncopyable {

    // 最多同时处理的 socket 数量
    static const int Max_Events;
    #ifdef NULL_EVENT_NOT_ALLOWED
    // 在 Linux 2.6.9 以下版本，EPOLL_CTL_DEL 不允许传入 event 空指针
    static epoll_event empty_event;
    #endif
    // 当前的 epoll fd
    int epoll_fd;
    // 存储 Epoll 返回 socket 的数组的长度
    int epoll_max_events;
    // 存储 Epoll 返回 socket 的数组
    std::shared_ptr<epoll_event> events;
    // 单例模式指针
    static std::shared_ptr<EpollManager> instance;
    
    EpollManager();

public:
    // 【默认】仅允许单线程处理同一链接，触发后即屏蔽响应
    static const __uint32_t Single_Time_Event;
    // 对于监听 socket 可以复用（因为只有主线程处理连接请求）
    static const __uint32_t Multi_Times_Event;
    
    /**
     * @brief 获取实例，如无实例则创建实例
    */
    static std::shared_ptr<EpollManager> getInstance();

    /**
     * @brief 初始化 EpollManager，否则无法正常使用
    */
    int init(int max_events);

    /**
     * @brief 添加监听的 socket fd
    */
    int addFd(int socket_fd, __uint32_t events);

    /**
     * @brief 修改已经添加的 socket fd
    */
    int modFd(int socket_fd, __uint32_t events);

    /**
     * @brief 删除已经添加的 socket fd
    */
    int delFd(int socket_fd);

    /**
     * @brief 异步等待 epoll 回调
     * @param ListenSocket 服务器监听 Socket
     * @param timeout 最长等待时间
     * @return 需要处理的连接 socket_fd 的数组（注意，新建连接的请求将在函数内被处理）
    */
    std::vector<int>
    poll(HttpServer &server, int timeout);
};