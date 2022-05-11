/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-10 07:48:27
 * @LastEditTime: 2022-05-11 14:23:00
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#pragma once
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <cstdint>
#include "MutexLock.h"
#include "Noncopyable.h"


class Condition: Noncopyable{
public:
    explicit Condition(MutexLock &_mutex): mutex(_mutex){
        pthread_cond_init(&cond, NULL);
    }
    ~Condition(){pthread_cond_destroy(&cond);}
    void wait() {pthread_cond_wait(&cond, mutex.get());}
    void notify(){pthread_cond_signal(&cond);}
    void notifyAll() {pthread_cond_broadcast(&cond);}
    bool waitForSeconds(int seconds){
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME,  &abstime);//函数"clock_gettime"是基于Linux C语言的时间函数,他可以用于计算精度和纳秒。//CLOCK_REALTIME:系统实时时间,随系统实时时间改变而改变,即从UTC1970-1-1 0:0:0开始计时
        abstime.tv_sec += static_cast<time_t>(seconds);
        return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);
    }
private:
    MutexLock &mutex;
    pthread_cond_t cond;
};
