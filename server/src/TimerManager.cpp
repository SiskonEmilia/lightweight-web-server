#include "timer/TimerManager.h"

void TimerManager::checkExpire() {
    MutexLockGuard(this->mutex);
    while (!timer_min_heap.empty()) {
        if (timer_min_heap.top()->checkExpire()) {
            // 从堆内移除被延迟删除标记的或过期的定时器
            timer_min_heap.pop();
        } else {
            // 没有更多要处理的节点，退出循环
            break;
        }
    }
}