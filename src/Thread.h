/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-10 07:42:45
 * @LastEditTime: 2022-05-11 18:42:13
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#pragma once
#include<pthread.h>
#include<functional>
#include<string>
#include <sys/syscall.h>
#include"mylib/Noncopyable.h"
#include"mylib/MutexLock.h"
#include"mylib/Condition.h"
#include"mylib/CountDownLatch.h"
#include"Reactor.h"

class ThreadPool;

class Thread: Noncopyable // 专用于Reactor, 对pthread_t进行封装
{
public:
    // Thread control
    typedef std::function<void()> ThreadFunc;
    explicit Thread( const std::string &name = std::string());
    ~Thread();
    void start();
    int join();
    bool started() const {return started_;}
    pid_t tid() const {return tid_;}
    const std::string& name() const {return name_;}

    //Reactor related
    Reactor* startLoop();
private:

    // Thread control
    void setDefaultName();
    bool started_;
    bool joined_;
    pthread_t pthreadId_; //unsigned long
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    CountDownLatch latch_;

    //Reactor related
    void threadFunc();
    Reactor *reactor_;
    bool exiting_;
    MutexLock mutex_;
    Condition cond_;

};