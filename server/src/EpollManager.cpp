#include <iostream>

#include "config/Configs.h"
#include "epoll/EpollManager.h"
#include "http/HttpServer.h"
#include "utils/Utils.h"

using std::cout;
using std::cerr;
using std::endl;

/* 初始化静态常量 */
const int EpollManager::Max_Events = 16384;
const __uint32_t EpollManager::Single_Time_Accept_Event = (EPOLLIN | EPOLLET | EPOLLONESHOT);
const __uint32_t EpollManager::Multi_Times_Accept_Event = (EPOLLIN | EPOLLET);
const __uint32_t EpollManager::Single_Time_Send_Event = (EPOLLOUT | EPOLLET | EPOLLONESHOT);

/* 初始化静态非常量 */
#ifdef NULL_EVENT_NOT_ALLOWED
epoll_event EpollManager::empty_event;
#endif
std::shared_ptr<EpollManager> EpollManager::instance = nullptr;

EpollManager::EpollManager() 
    : epoll_fd(-1), epoll_max_events(0),
      events(nullptr, Utils::arrayDeleter<epoll_event>) { }

std::shared_ptr<EpollManager> EpollManager::getInstance() {
    if (instance) { 
        return instance;
    } else {
        std::shared_ptr<EpollManager> temp(new EpollManager());
        instance.swap(temp);
        return instance;
    }
}

int EpollManager::init(int max_events) {
    if (epoll_fd != -1) {
        cout << "EpollManager: Try to initialize EpollManager for multiple times" << endl;
        return 0;
    }
    if (max_events <= 0 || max_events > Max_Events) {
        cout << "EpollManager: Invalid max events: " << max_events << endl
             << "EpollManager: Set to default value: " << Max_Events << endl;
        epoll_max_events = Max_Events;
    } else {
        cout << "EpollManager: Epoll max events: " << max_events << endl;
        epoll_max_events = max_events;
    }
    epoll_fd = epoll_create(epoll_max_events);
    if (epoll_fd == -1) {
        cerr << "EpollManager: Fail to create epoll instance in file <"
             << __FILE__ << "> "<< "at " << __LINE__  << endl;
        exit(-1);
    }
    std::shared_ptr<epoll_event> temp_events(new epoll_event[epoll_max_events], Utils::arrayDeleter<epoll_event>);
    // C++ 现在通过抛出异常而非返回空指针来指示内存分配错误
    // 所以无需判断空指针，此时已经是分配成功
    events.swap(temp_events);
    return 0;
}

int EpollManager::addFd(int socket_fd, __uint32_t events) {
    if (epoll_fd < 0) {
        cout << "EpollManager: Trying to add socket_fd without initializing. Call init() first." << endl;
        return -1;
    }
    epoll_event event;
    bzero(&event, sizeof(event));
    event.events = events;
    event.data.fd = socket_fd;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
    if (ret < 0) {
        #ifdef ENABLE_LOG
        cout << "EpollManager: Failed to add socket_fd to epoll instance." << endl;
        #endif
        return -1;
    }
    return 0;
}

int EpollManager::modFd(int socket_fd, __uint32_t events) {
    if (epoll_fd < 0) {
        cout << "EpollManager: Trying to modify socket_fd without initializing. Call init() first." << endl;
        return -1;
    }
    epoll_event event;
    bzero(&event, sizeof(event));
    event.events = events;
    event.data.fd = socket_fd;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd, &event);
    if (ret < 0) {
        #ifdef ENABLE_LOG
        cout << "EpollManager: Failed to modify epoll instance, all data remained unchanged." << endl;
        #endif
        return -1;
    }
    return 0;
}

int EpollManager::delFd(int socket_fd) {
    if (epoll_fd < 0) {
        cout << "EpollManager: Trying to delete socket_fd without initializing. Call init() first." << endl;
        return -1;
    }
    #ifdef NULL_EVENT_NOT_ALLOWED
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, empty_event);
    #else
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, nullptr);
    #endif
    if (ret < 0) {
        #ifdef ENABLE_LOG
        cout << "EpollManager: Failed to delete socket inside epoll instance, all data remained unchanged." << endl;
        #endif
        return -1;
    }
    return 0;
}

std::vector<int>
EpollManager::poll(HttpServer &server, int timeout) {
    if (epoll_fd < 0) {
        cout << "EpollManager: Trying to use epoll without initializing. Call init() first." << endl;
        return {};
    }
    if (timeout <= 0) {
        cout << "EpollManager: Trying to set invalid timeout: " << timeout << endl;
        return {};
    }
    int event_count = epoll_wait(epoll_fd, events.get(), epoll_max_events, timeout);
    if (event_count < 0) {
        cout << "EpollManager: epoll_wait() returned with error in file <"
             << __FILE__ << "> "<< "at " << __LINE__  << endl
             << "ERRCODE: " << errno << endl;
        exit(-1);
    }

    std::vector<int> connections;
    for (int i = 0; i < event_count; ++i) {
        int socket_fd = events.get()[i].data.fd;

        if (socket_fd == server.getListenSocketFd()) {
            // 有向监听端口发来的连接请求，创建连接
            server.handleConnection(timeout);
        } else {
            if (events.get()[i].events & (EPOLLERR | EPOLLRDHUP | EPOLLHUP)) {
                // 出错/半关闭/无效的连接
                server.removeConnection(events.get()[i].data.fd);
                // 下面这行由服务器代为完成
                // delFd(events.get()[i].data.fd);
            } else {
                connections.push_back(events.get()[i].data.fd);
            }
        }
    }
    
    return connections;
}