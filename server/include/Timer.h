#pragma once

#include <sys/time.h>
#include <memory>

/**
 * @brief 定时器基类，需定制派生类并配合 TimeManager 使用
 * 通过实现纯虚函数 handleExpire 来实现超时回调
*/
class Timer {

    bool deleted;
    time_t expired_time;

    /**
     * @brief 超时时调用的回调函数，派生类继承此函数来实现超时回调
    */
    virtual void handleExpire() = 0;

public:
    static time_t current_time;

    typedef std::shared_ptr<Timer> TimerSPtr;

    struct Earlier {
        bool operator()(const TimerSPtr &a, const TimerSPtr &b) {
            return eailer(*a, *b);
        }
    };

    struct Later {
        bool operator()(const TimerSPtr &a, const TimerSPtr &b) {
            return later(*a, *b);
        }
    };

    /**
     * @brief 根据超时时间创建 Timer，注意单位为毫秒
    */
    explicit Timer(time_t timeout) : deleted(false) {
        updateCurrentTime();
        expired_time = timeout + current_time;
    }

    /**
     * @brief 虚析构函数，避免派生类被部分析构
    */
    virtual ~Timer() { }

    bool isDeleted() const { return deleted; }

    time_t getExpireTime() const { return expired_time; }

    /**
     * @brief 返回定时器是否超时。同时，如果定时器超时且有效则执行回调函数
    */
    bool checkExpire();

    /**
     * @brief 标记延迟删除
    */
    void deleteTimer() { deleted = true; }

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
    static bool eailer(const Timer& lhs, const Timer& rhs) { return lhs.expired_time < rhs.expired_time; }

    /**
     * @brief 若左侧操作数更晚超时，返回 true
    */
    static bool later(const Timer& lhs, const Timer& rhs) { return lhs.expired_time > rhs.expired_time; }
};
