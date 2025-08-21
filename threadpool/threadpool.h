#ifndef __THREADPOOL__
#define __THREADPOOL__

#include <pthread.h>

/*互斥锁类*/
class Locker 
{
public:
    Locker();

    ~Locker();

    bool lock();

    bool unlock();

    pthread_mutex_t *get_mutex();

private:
    pthread_mutex_t m_mutex;

};

/*线程池类*/
class ThreadPool 
{
public:
    ThreadPool();

    ~ThreadPool();

private:
    // 创建线程时指定的运行函数，参数传递 this，实现在子线程中可以访问到该对象的成员
    static void *worker(void *arg);
    
    // 在线程中执行该函数等待处理事件队列中的事件
    void run();
private:
    int m_thread_num;       //池中线程个数
    pthread_t *m_threads;   //保存所有线程的ID

};

#endif // __THREADPOOL__