#pragma once

#include <queue>
#include <vector>
#include <memory>

#include "timer/Timer.h"
#include "thread/MutexLock.h"

class TimerManager {

    // 堆互斥锁
    MutexLock mutex;
    std::priority_queue<Timer::TimerSPtr, std::vector<Timer::TimerSPtr>, Timer::Later> timer_min_heap;

public:
    /**
     * @brief 通过智能指针添加 timer，外部应只保存 weak_ptr，防止 timer 未被正常析构
    */
    void append(Timer::TimerSPtr timer) {
        // 由于多个线程都有可能重置定时器，因此需要加锁
        MutexLockGuard guard(this->mutex);
        timer_min_heap.push(timer);
    }

    /**
     * @brief 检查小根堆，循环执行堆顶超时计时器的回调函数并删除堆顶无效计时器，
     * 直到堆顶为合法的未超时计时器
    */
    void checkExpire();
};