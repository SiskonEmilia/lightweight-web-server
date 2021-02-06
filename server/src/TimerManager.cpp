#include "TimerManager.h"

void TimerManager::checkExpire() {
    MutexLockGuard(this->mutex);
    while (!timer_min_heap.empty()) {
        Timer::TimerSPtr temp_ptr = timer_min_heap.top();
        if (temp_ptr->isDeleted() || temp_ptr->checkExpire()) {
            // 被延迟删除标记的节点，或过期的定时器，从堆内移除
            timer_min_heap.pop();
        } else {
            // 没有更多要处理的节点，退出循环
            break;
        }
    }
}