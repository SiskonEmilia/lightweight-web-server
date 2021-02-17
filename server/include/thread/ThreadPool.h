#pragma once

#include <vector>
#include <list>
#include <functional>
#include <pthread.h>
#include <memory>
#include <iostream>
#include <pthread.h>
#include <sys/prctl.h>
#include <csignal>

#include "utils/Uncopyable.h"
#include "thread/MutexLock.h"
#include "thread/Condition.h"

template<typename T>
struct ThreadTask {

    std::function<void(std::shared_ptr<T>)> process; // 线程接受的函数
    std::shared_ptr<T> arg;                          // 该函数接受的参数表指针（若为单参数，则可为参数指针）

    ThreadTask() {}
    ThreadTask(std::shared_ptr<T>& arg, std::function<void(std::shared_ptr<T>)>& process):
       process(process), arg(arg) { } 
};

template<typename T>
class ThreadPool {

    // 互斥锁
    MutexLock mutex;
    Condition condition;
    // 线程池属性
    int thread_num;
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
        Default_Thread_Num = 8,
        Max_Thread_Num = 1024,
        Max_Queue_Size = 16384
    };

    typedef enum {
        immediate_mode = 1,
        graceful_mode = 2
    } ShutdownMode;

    ThreadPool(int thread_num, int max_queue_size);

    ~ThreadPool();

    bool append(std::shared_ptr<T> arg, std::function<void(std::shared_ptr<T>)> fun);

    void shutdown(bool graceful);
};

using std::cout;
using std::cerr;
using std::endl;

template<typename T>
ThreadPool<T>::ThreadPool(int thread_num, int max_queue_size) : 
    thread_num(thread_num), max_queue_size(max_queue_size), 
    started(0), shutdown_(0) {

    if (thread_num <= 0 || thread_num > Max_Thread_Num) {
        // 不合法的线程数量
        cout << "Invalid thread size: " << thread_num << ", " << endl
             << "will be set to default value: " << Default_Thread_Num << endl;
        thread_num = Default_Thread_Num;
    }

    if (max_queue_size <= 0 || max_queue_size > Max_Queue_Size) {
        // 不合法的最大任务数量
        cout << "Invalid waiting queue size: " << max_queue_size << ", " << endl
             << "will be set to default value: " << Max_Queue_Size << "." << endl;
        max_queue_size = Max_Queue_Size;
    }

    // 创建线程，并保存线程的 tid
    threads.resize(thread_num);
    for (int i = 0; i < thread_num; i++) {
        if (pthread_create(&threads[i], NULL, worker, this) != 0) {
            cerr << "Failed to create thread#" << i << "." << endl;
            abort();
        }
        started++;
    }
}

template<typename T>
ThreadPool<T>::~ThreadPool() {
    if (!shutdown_)
        shutdown(true);
}

template<typename T>
bool ThreadPool<T>::append(std::shared_ptr<T> arg, std::function<void(std::shared_ptr<T>)> fun) {
    // 如果已经停机，则不接受更多的链接
    if (shutdown_) {
        cout << "Failed to append task: ThreadPool has been shutdown." << endl;
        return false;
    }

    MutexLockGuard guard(this->mutex);
    if (request_queue.size() >= max_queue_size) {
        cout << "Failed to append task: waiting queue is full." << endl;
        return false;
    }
    
    // 向等待队列添加新的任务
    request_queue.push_back({arg, fun});
    // 通知空闲的线程处理任务队列
    condition.notify();
    return true;
}

template<typename T>
void ThreadPool<T>::shutdown(bool graceful) {
    {
        MutexLockGuard guard(this->mutex);
        // 如果已经在停机状态，不需额外操作
        if (shutdown_) {
            cout << "Fail to shutdown: server has been shutdown." << endl;
            // 进程退出时自动释放持有的文件符，无需手动释放
            return;
        }

        shutdown_ = graceful ? graceful_mode : immediate_mode;
        condition.notifyAll();
    }
    // 等待所有 thread 执行完毕
    for (int i = 0; i < thread_num; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            cerr << "Fail to end thread: pthread_join error for thread#" << i << endl
                 << "-- TID: " << threads[i] << endl;
        }
    }
}

template<typename T>
void *ThreadPool<T>::worker(void *args) {
    ThreadPool *pool = static_cast<ThreadPool *>(args);
    // 若线程池指针无效，则立即停止执行
    if (pool == nullptr)
        return NULL;
    // 设置线程名
    prctl(PR_SET_NAME,"EventLoopThread");

    sigset_t cur_set;
    sigaddset(&cur_set, SIGPIPE);
    sigaddset(&cur_set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &cur_set, nullptr);

    // 执行 EventLoop
    pool->run();
    return NULL;
}

template<typename T>
void ThreadPool<T>::run() {
    // EventLoop
    while (true) {
        ThreadTask<T> requestTask;
        {
            // 此作用域用来约束互斥锁范围
            MutexLockGuard guard(this->mutex);
            // 没有任务且没有停机的情况下，不断通过条件变量进行等待（自旋锁会不会更快？）
            while (request_queue.empty() && !shutdown_) {
                // 此处条件变量会解锁互斥锁，因而不会死锁
                condition.wait(this->mutex);
            }

            // 处理停机的情况：如果立即停机，则不处理剩下任务
            // 如果为优雅关闭模式，则继续处理剩下的任务，直到处理完停机
            if ((shutdown_ == immediate_mode) || (shutdown_ == graceful_mode && request_queue.empty())) {
                break;
            }
            
            // 处理等待队列中的任务
            requestTask = request_queue.front();
            request_queue.pop_front();
        }
        // 此时互斥锁已经释放，因而执行过程不会阻塞线程池
        requestTask.process(requestTask.arg);
    }
}