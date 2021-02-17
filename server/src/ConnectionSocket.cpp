#include <iostream>

#include "sockets/ConnectionSocket.h"
#include "timer/HttpTimer.h"
#include "timer/TimerManager.h"
#include "http/HttpServer.h"

using std::cout;
using std::endl;

bool ConnectionSocket::setTimer(const time_t timeout) {
    if (timeout <= 0) {
        cout << "ConnectionSocket: Trying to set invalid timeout." << endl;
        return false;
    }
    if (timer.lock()) {
        cout << "ConnectionSocket: Timer existed, remove it first." << endl;
        return false;
    }
    timer = server.createTimer(timeout, getSocketFd());
    return true;
}

void ConnectionSocket::removeTimer() {
    std::shared_ptr<HttpTimer> old_timer = timer.lock();
    if (old_timer) {
        old_timer->checkAndSetDelete();
        old_timer.reset();   
    }
    timer.reset();
}

void ConnectionSocket::expireHandler() {
    int socket_fd = getSocketFd();
    // 关闭 socket 文件符
    Socket::close();
    // 通知服务器删除文件描述符
    server.removeConnection(socket_fd);
}


/**
 * Q1：为什么 Line 25 的 lock() 不需要同步？
 * A1：因为此操作会在主线程调用，与处理超时操作同线程，不会有顺序问题。
 *    EventLoop 会首先标记删除所有即将处理的 Timer，然后再将他们送入
 *    线程池。
*/