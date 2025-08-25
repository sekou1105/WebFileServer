/*************************************************************
*循环数组实现的阻塞队列，m_back = (m_back + 1) % m_max_size;  
*线程安全，每个操作前都要先加互斥锁，操作完后，再解锁
**************************************************************/
#ifndef __BLOCK_QUEUE__
#define __BLOCK_QUEUE__

#include <iostream>
#include <stdlib.h>
#include "../threadpool/threadpool.h"

template <class T>
class BlockQueue 
{
public:
    BlockQueue(int max_size = 1000);

    ~BlockQueue();

    //队列是否满
    bool full();
    //队列是否空
    bool empty();
    //出队
    bool pop(T &item);
    //入队
    bool push(const T &item);
    //获取队首元素
    bool front(T &value);
    //获取队尾元素
    bool back(T &value);
    //获取当前队列大小
    int size();
    //队列最大大小
    int max_size();
    //清空队列
    void clear();

private:
    Locker m_mutex;
    Cond m_cond;

    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};

// ================= Template Implementations =================
template <class T>
BlockQueue<T>::BlockQueue(int max_size)
    : m_array(nullptr), m_size(0), m_max_size(max_size), m_front(-1), m_back(-1) {
    if (m_max_size <= 0) {
        m_max_size = 1;
    }
    m_array = new T[m_max_size];
}

template <class T>
BlockQueue<T>::~BlockQueue() {
    m_mutex.lock();
    if (m_array) {
        delete[] m_array;
        m_array = nullptr;
    }
    m_mutex.unlock();
}

template <class T>
bool BlockQueue<T>::full() {
    m_mutex.lock();
    bool is_full = (m_size >= m_max_size);
    m_mutex.unlock();
    return is_full;
}

template <class T>
bool BlockQueue<T>::empty() {
    m_mutex.lock();
    bool is_empty = (m_size <= 0);
    m_mutex.unlock();
    return is_empty;
}

template <class T>
bool BlockQueue<T>::pop(T &item) {
    m_mutex.lock();
    while (m_size <= 0) {
        if (!m_cond.wait(m_mutex.get_mutex())) {
            m_mutex.unlock();
            return false;
        }
    }
    m_front = (m_front + 1) % m_max_size;
    item = m_array[m_front];
    m_size--;
    m_mutex.unlock();
    return true;
}

template <class T>
bool BlockQueue<T>::push(const T &item) {
    m_mutex.lock();
    if (m_size >= m_max_size) {
        m_cond.broadcast();
        m_mutex.unlock();
        return false;
    }
    m_back = (m_back + 1) % m_max_size;
    m_array[m_back] = item;
    m_size++;
    m_cond.broadcast();
    m_mutex.unlock();
    return true;
}

template <class T>
bool BlockQueue<T>::front(T &value) {
    m_mutex.lock();
    if (m_size == 0) {
        m_mutex.unlock();
        return false;
    }
    int index = (m_front + 1) % m_max_size;
    value = m_array[index];
    m_mutex.unlock();
    return true;
}

template <class T>
bool BlockQueue<T>::back(T &value) {
    m_mutex.lock();
    if (m_size == 0) {
        m_mutex.unlock();
        return false;
    }
    value = m_array[m_back];
    m_mutex.unlock();
    return true;
}

template <class T>
int BlockQueue<T>::size() {
    m_mutex.lock();
    int tmp = m_size;
    m_mutex.unlock();
    return tmp;
}

template <class T>
int BlockQueue<T>::max_size() {
    m_mutex.lock();
    int tmp = m_max_size;
    m_mutex.unlock();
    return tmp;
}

template <class T>
void BlockQueue<T>::clear() {
    m_mutex.lock();
    m_size = 0;
    m_front = -1;
    m_back = -1;
    m_mutex.unlock();
}

#endif