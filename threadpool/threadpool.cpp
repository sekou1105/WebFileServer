#include "threadpool.h"
#include "../event/myevent.h"

Locker::Locker() {
    if (pthread_mutex_init(&m_mutex, NULL) != 0)
    {
        throw std::exception();
    }
}

Locker::~Locker() {
    pthread_mutex_destroy(&m_mutex);
}

bool Locker::lock() {
    return pthread_mutex_lock(&m_mutex) == 0;
}

bool Locker::unlock() {
    return pthread_mutex_unlock(&m_mutex) == 0;
}

pthread_mutex_t *Locker::get_mutex() {
    return &m_mutex;
}

//线程池
ThreadPool::ThreadPool(int threadNum):m_thread_num(threadNum) {
    int ret = 0;
    //初始化线程池中的线程
    m_threads = new pthread_t[m_thread_num];
    for(int i = 0; i < m_thread_num; ++i) {
        ThreadArgs* args = new ThreadArgs;
        args->pool = this;
        args->threadId = i;
        //创建线程
        ret = pthread_create(m_threads + i, nullptr, worker, args);
        if(ret != 0) {
            LOG_ERROR("failed to create thread: %d", errno);
            delete[] m_threads;
            delete args;
            throw std::runtime_error("线程创建失败");
        }
        //分离线程
        ret = pthread_detach(m_threads[i]);
        if(ret != 0){
            delete args;
            delete[] m_threads;
            throw std::runtime_error("设置脱离线程失败");
        }
    }
}

ThreadPool::~ThreadPool() {
    // 释放动态创建的保存线程 id 的数组
    delete[] m_threads;
}

int ThreadPool::appendEvent(EventBase* event, const std::string eventType){
    int ret = 0;
    // 事件队列加锁
    ret = queueLocker.lock();
    if(!ret){
        LOG_ERROR("事件队列加锁失败");
        return -1;
    }
    // 向队列中添加事件
    m_workQueue.push(event);
    LOG_INFO("添加成功，线程池事件队列中剩余的事件个数：%d", m_workQueue.size());
    
    // 事件队列解锁
    ret = queueLocker.unlock();
    if(!ret){
        LOG_ERROR("事件队列解锁失败");
        return -2;
    }
    // 事件队列的信号量加一

    ret = queueEventSem.post();
    if(ret != 0){
        LOG_ERROR("事件队列信号量 post 失败");
        return -3;
    }
    
    return 0;
}

void *ThreadPool::worker(void *arg){
    //获取线程参数，显示转换 void*类型 -> ThreadArgs*类型
    ThreadArgs *args = static_cast<ThreadArgs*>(arg);
    ThreadPool *pool = args->pool;
    int threadId = args->threadId;

    //delete线程参数结构体
    delete args;

    //调用线程池run函数
    pool->run(threadId);

    return nullptr;

}

void ThreadPool::run(int threadId){
    LOG_INFO("线程 %d 正在执行", threadId);
    while(1){
        bool ret = queueEventSem.wait();
        if(!ret) {
            LOG_ERROR("等待队列事件失败");
            return;
        }
        LOG_INFO("线程 %d 收到事件", threadId);

        ret = queueLocker.lock();
        if(!ret){
            LOG_ERROR("事件队列加锁失败");
            return;
        }
        //获取队列前面的事件
        EventBase *curEvent = m_workQueue.front();
        m_workQueue.pop();

        ret = queueLocker.unlock();
        if(!ret){
            LOG_ERROR("事件队列解锁失败");
            return;
        }

        if(curEvent == nullptr){
            continue;
        }
        LOG_INFO("线程 %d 开始处理事件", threadId);
        curEvent->process();
        LOG_INFO("线程 %d 处理事件完成", threadId);
        delete curEvent;
    }
}
