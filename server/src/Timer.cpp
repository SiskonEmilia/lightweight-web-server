#include "timer/Timer.h"

time_t Timer::current_time = 0;

bool Timer::checkExpire() {
    // 不在此函数更新当前时间，以避免频繁的系统调用
    MutexLockGuard(this->mutex);
    if (deleted) {
        return true;
    } else if (expired_time < current_time) {
        // 调用虚函数，使得派生类的超时回调能够生效
        handleExpire();
        // 标记延迟删除
        deleted = true;
        return true;
    } else {
        return false;
    }
}