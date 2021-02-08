#pragma once

#include <pthread.h>

#include "utils/Uncopyable.h"

/**
 * @brief 封装互斥锁
*/
class MutexLock : private Uncopyable {

    pthread_mutex_t raw_mutex;

public:
    MutexLock() {
        pthread_mutex_init(&raw_mutex, nullptr);
    }
    ~MutexLock() {
        pthread_mutex_destroy(&raw_mutex);
    }
    void lock() {
        pthread_mutex_lock(&raw_mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&raw_mutex);
    }
    pthread_mutex_t* getMutex() {
        return &raw_mutex;
    }
};

/**
 * @brief 以 RAII 手法进行互斥锁加解锁管理
*/
class MutexLockGuard : private Uncopyable {

    MutexLock &mutex;

public:
    explicit MutexLockGuard(MutexLock &mutex) : mutex(mutex) {
        mutex.lock();
    }

    ~MutexLockGuard() {
        mutex.unlock();
    }
};