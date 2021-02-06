#include <iostream>
#include <pthread.h>
#include <sys/prctl.h>

#include "../include/ThreadPool.h"

using std::cout;
using std::cerr;
using std::endl;

ThreadPool::ThreadPool(int thread_size, int max_queue_size) : 
    max_queue_size(max_queue_size), thread_size(thread_size),
    started(0), shutdown_(0) {

    if (thread_size <= 0 || thread_size > Max_Thread_Size) {
        // 不合法的线程数量
        cout << "Invalid thread size: " << thread_size << ", " << endl
             << "will be set to default value: 4." << endl;
        thread_size = 4;
    }

    if (max_queue_size <= 0 || max_queue_size > Max_Queue_Size) {
        // 不合法的最大任务数量
        cout << "Invalid waiting queue size: " << max_queue_size << ", " << endl
             << "will be set to default value: " << Max_Queue_Size << "." << endl;
        max_queue_size = Max_Queue_Size;
    }

    // 创建线程，并保存线程的 tid
    threads.resize(thread_size);
    for (int i = 0; i < thread_size; i++) {
        if (pthread_create(&threads[i], NULL, worker, this) != 0) {
            cerr << "Failed to create thread#" << i << "." << endl;
            abort();
        }
        started++;
    }
}

ThreadPool::~ThreadPool() {
    if (!shutdown_)
        shutdown(true);
}

bool ThreadPool::append(std::shared_ptr<void> arg, std::function<void(std::shared_ptr<void>)> fun) {
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

void ThreadPool::shutdown(bool graceful) {
    {
        MutexLockGuard guard(this->mutex);
        // 如果已经在停机状态，不需额外操作
        if (shutdown_) {
            cout << "Fail to shutdown: dumplicate shutdown." << endl;
            return;
        }

        shutdown_ = graceful ? graceful_mode : immediate_mode;
        condition.notifyAll();
    }
    // 等待所有 thread 执行完毕
    for (int i = 0; i < thread_size; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            cerr << "Fail to end thread: pthread_join error for thread#" << i << endl
                 << "-- TID: " << threads[i] << endl;
        }
    }
}

void *ThreadPool::worker(void *args) {
    ThreadPool *pool = static_cast<ThreadPool *>(args);
    // 若线程池指针无效，则立即停止执行
    if (pool == nullptr)
        return NULL;
    // 设置线程名
    prctl(PR_SET_NAME,"EventLoopThread");

    // 执行 EventLoop
    pool->run();
    return NULL;
}

void ThreadPool::run() {
    // EventLoop
    while (true) {
        ThreadTask requestTask;
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
