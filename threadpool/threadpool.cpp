#include "threadpool.h"

Locker::Locker() {
    pthread_mutex_init(&m_mutex, NULL);
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
