#pragma once

#include <sys/time.h>
#include <memory>

#include "thread/MutexLock.h"

/**
 * @brief 线程安全的定时器基类，需定制派生类并配合 TimeManager 使用
 * 通过实现纯虚函数 handleExpire 来实现超时回调
*/
class Timer {

    bool deleted;
    time_t expired_time;
    // 互斥锁，或许应该换用轻量的自旋锁？
    MutexLock mutex;

    /**
     * @brief 超时时调用的回调函数，派生类继承此函数来实现超时回调
    */
    virtual void handleExpire() = 0;

    // 禁止默认构造
    Timer();

public:
    static time_t current_time;

    typedef std::shared_ptr<Timer> TimerSPtr;

    /**
     * @brief 封装 earlier 比较函数的可调用结构体
    */
    struct Earlier {
        bool operator()(const TimerSPtr &a, const TimerSPtr &b) {
            return earlier(*a, *b);
        }
    };

    /**
     * @brief 封装 later 比较函数的可调用结构体
    */
    struct Later {
        bool operator()(const TimerSPtr &a, const TimerSPtr &b) {
            return later(*a, *b);
        }
    };

    /**
     * @brief 根据超时时间创建 Timer，注意单位为毫秒
     * @param timeout: 以毫秒计的超时时间
    */
    explicit Timer(const time_t timeout) 
            : deleted(false){
        updateCurrentTime();
        expired_time = timeout + current_time;
    }

    /**
     * @brief 虚析构函数，避免派生类被部分析构
    */
    virtual ~Timer() { }

    /**
     * @brief 【线程安全】检查延迟删除状态并设置延迟删除
    */
    virtual bool checkAndSetDelete() { 
        MutexLockGuard(this->mutex);
        bool old_deleted = deleted;
        deleted = true;
        return old_deleted;
    }

    time_t getExpireTime() const { return expired_time; }

    /**
     * @brief 【线程安全】返回定时器是否超时。同时，如果定时器超时且有效则执行回调函数
    */
    bool checkExpire();

    /**
     * @brief 通过系统调用更新当前时间
    */
    static void updateCurrentTime() {
        struct timeval current_timeval;
        gettimeofday(&current_timeval, nullptr);
        current_time = current_timeval.tv_sec * 1000 + current_timeval.tv_usec / 1000;
    }

    /**
     * @brief 若左侧操作数更早超时，返回 true
    */
    static bool earlier(const Timer& lhs, const Timer& rhs) { return lhs.expired_time < rhs.expired_time; }

    /**
     * @brief 若左侧操作数更晚超时，返回 true
    */
    static bool later(const Timer& lhs, const Timer& rhs) { return lhs.expired_time > rhs.expired_time; }
};

/**
 * Timer 线程安全的重点在于，防止 Timer 正准备执行超时回调时，外部设置延迟删除
*/