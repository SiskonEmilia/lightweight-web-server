#pragma once

#include <pthread.h>

#include "base/Uncopyable.h"

class MutexLock : private Uncopyable {

    pthread_mutex_t mutex_;

public:
    MutexLock() {
        pthread_mutex_init(&mutex_, nullptr);
    }
    ~MutexLock() {
        pthread_mutex_destroy(&mutex_);
    }
    void lock() {
        pthread_mutex_lock(&mutex_);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }
    pthread_mutex_t* getMutex() {
        return &mutex_;
    }
};

// RAII mutex lock manager class
class MutexLockAuto : private Uncopyable {

    MutexLock &raw_mutex;

public:
    explicit MutexLockAuto(MutexLock &raw_mutex) : raw_mutex(raw_mutex) {
        raw_mutex.lock();
    }

    ~MutexLockAuto() {
        raw_mutex.unlock();
    }
};