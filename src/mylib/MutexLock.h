/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-10 07:48:50
 * @LastEditTime: 2022-05-10 08:37:45
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#pragma once
#include<pthread.h>
#include"Noncopyable.h"
#include"Condition.h"


class MutexLock: Noncopyable
{
    // 友元类不受访问权限影响
    friend class Condition;

public:
    MutexLock() { pthread_mutex_init(&mutex, NULL); }
    ~MutexLock(){
        pthread_mutex_lock(&mutex);
        pthread_mutex_destroy(&mutex);
    }
    void lock() { pthread_mutex_lock(&mutex); }
    void unlock() { pthread_mutex_unlock(&mutex); }
    pthread_mutex_t *get() { return &mutex; }

private:
    pthread_mutex_t mutex;
};

class MutexLockGuard: Noncopyable{
public:
    explicit MutexLockGuard(MutexLock &_mutex) : mutex(_mutex) { mutex.lock(); }
    ~MutexLockGuard() { mutex.unlock(); }

private: 
    MutexLock &mutex;
};
