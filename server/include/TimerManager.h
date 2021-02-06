#pragma once

#include <queue>
#include <vector>
#include <memory>

#include "Timer.h"
#include "MutexLock.h"

class TimerManager {

    MutexLock mutex;
    std::priority_queue<Timer::TimerSPtr, std::vector<Timer::TimerSPtr>, Timer::Later> timer_min_heap;

public:
    void append(Timer::TimerSPtr &timer) {
        // 由于多个线程都有可能重置定时器，因此需要加锁
        MutexLockGuard(this->mutex);
        timer_min_heap.push(timer);
    }

    /**
     * @brief 检查小根堆，循环执行堆顶超时计时器的回调函数并删除堆顶无效计时器，
     * 直到堆顶为合法的未超时计时器
    */
    void checkExpire();
};