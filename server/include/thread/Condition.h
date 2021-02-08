/**
 * 封装多线程条件变量的类
*/

#pragma once

#include <pthread.h>

#include "utils/Uncopyable.h"
#include "thread/MutexLock.h"

class Condition : private Uncopyable{

    pthread_cond_t condition;

public:
    /**
     * @brief Use mutex lock to initialize condition.
    */
    explicit Condition() {
        pthread_cond_init(&condition, NULL);
    }

    ~Condition() {
        pthread_cond_destroy(&condition);
    }

    void wait(MutexLock& mutex) {
        // 注意：pthread_cond_wait 会解锁互斥锁，并在被唤醒时重新加锁
        pthread_cond_wait(&condition, mutex.getMutex());
    }

    void notify() {
        pthread_cond_signal(&condition);
    }

    void notifyAll() {
        pthread_cond_broadcast(&condition);
    }
};