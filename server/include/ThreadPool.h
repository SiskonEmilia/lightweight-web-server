#pragma once

#include <vector>
#include <list>
#include <functional>
#include <pthread.h>
#include <memory>


#include "base/Uncopyable.h"
#include "MutexLock.h"
#include "Condition.h"

template<typename T>
struct ThreadTask {

    std::function<void(std::shared_ptr<T>)> process; // 线程接受的函数
    std::shared_ptr<T> arg;                          // 该函数接受的参数表指针（若为单参数，则可为参数指针）

    ThreadTask() {}
    ThreadTask(std::shared_ptr<T>& arg, std::function<void(std::shared_ptr<T>)>& process):
        arg(arg), process(process) { } 
};

template<typename T>
class ThreadPool {

    // 互斥锁
    MutexLock mutex;
    Condition condition;
    // 线程池属性
    int thread_size;
    int max_queue_size;
    // 状态信息
    int started;
    int shutdown_;
    // 资源池与请求队列
    std::vector<pthread_t> threads;
    std::list<ThreadTask<T>> request_queue;

    static void *worker(void *args);
    
    /**
     * @brief EventLoop：执行执行等待队列内的任务。
    */
    void run();

public:
    enum {
        Max_Thread_Size = 1024,
        Max_Queue_Size = 16384
    };

    typedef enum {
        immediate_mode = 1,
        graceful_mode = 2
    } ShutdownMode;

    ThreadPool(int thread_size, int max_queue_size);

    ~ThreadPool();

    bool append(std::shared_ptr<T> arg, std::function<void(std::shared_ptr<T>)> fun);

    void shutdown(bool graceful);
};


//#endif //WEBSERVER_THREADPOLL_H
