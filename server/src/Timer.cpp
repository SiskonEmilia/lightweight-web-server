#include "Timer.h"

time_t Timer::current_time = 0;

bool Timer::checkExpire() {
    // 不在此函数更新当前时间，以避免频繁的系统调用
    if (expired_time < current_time) {
        if (!deleted) {
            handleExpire();
            deleteTimer();
        }
        return true;
    } 
    return false;
}