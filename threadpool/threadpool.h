#ifndef __THREADPOOL__
#define __THREADPOOL__

#include <semaphore.h>
#include <pthread.h>
#include <string>
#include <exception>
#include <queue>

// 前向声明，避免在头文件中引入 myevent.h 造成循环依赖
class EventBase;

class ThreadPool;
// 线程参数结构体，用于传递ThreadPool指针和线程序号
struct ThreadArgs {
    ThreadPool* pool;
    int threadId;
};

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

//信号量
class Sem
{
public:
    Sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    Sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~Sem()
    {
        sem_destroy(&m_sem);
    }
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

/*线程池类*/
class ThreadPool 
{
public:
    ThreadPool(int threadNum);

    ~ThreadPool();
public:
    // 向事件队列中添加一个待处理的事件，线程池中的线程会循环处理其中的事件
    int appendEvent(EventBase* event, const std::string eventType);

private:
    // 创建线程时指定的运行函数，参数传递 this，实现在子线程中可以访问到该对象的成员
    static void *worker(void *arg);
    
    // 在线程中执行该函数等待处理事件队列中的事件
    void run(int threadId);
private:
    int m_thread_num;       //池中线程个数
    pthread_t *m_threads;   //保存所有线程的ID

    std::queue<EventBase*> m_workQueue;  // 保存所有待处理的事件
    Locker queueLocker;     // 用于互斥访问事件队列的锁
    Sem queueEventSem;             // 表示队列中事件个数变化的信号量

};

#endif // __THREADPOOL__