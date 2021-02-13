#pragma once

#include <memory>
#include <iostream>

#include "timer/Timer.h"
#include "sockets/ConnectionSocket.h"

/**
 * @brief Http 服务器特化的计时器类
 * 【此为测试版本，不提供实际服务】
*/
class HttpTimer : public Timer {

    std::shared_ptr<ConnectionSocket> socket;

    // 禁止默认构造
    HttpTimer();

    /**
     * @brief 移除计时器与资源的绑定
    */
    void removeBinding() {
        socket.reset();
    }

    virtual void handleExpire() override {
        socket->expireHandler();
        removeBinding();
    }

public:

    explicit HttpTimer(const time_t timeout, std::shared_ptr<ConnectionSocket> socket) 
        : Timer(timeout), socket(socket) { }

    /**
     * @brief 【线程安全】检查延迟删除状态并设置延迟删除
    */
    virtual bool checkAndSetDelete() override {
        bool old_delete = Timer::checkAndSetDelete();
        if (!old_delete) {
            removeBinding();
        } else { }
        return old_delete;
    }
};