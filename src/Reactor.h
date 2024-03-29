/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-11 08:41:15
 * @LastEditTime: 2022-06-13 00:39:24
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#pragma once
#include"mylib/MutexLock.h"
#include"Epoll.h"
#include"mylib/TimerQueue.h"
#include<functional>
#include<vector>
#include<memory>
class HttpHandler;
class Reactor{
public:
    typedef std::function<void()> Functor;
    Reactor();
    ~Reactor();
    void quit();
    void loop();
    void runInLoop(Functor&&);
    void queueInLoop(Functor&&);
    void wakeup();//当前线程唤醒该Reactor对应的子线程

private:
    bool looping_; // 是否处于Reactor::loop()过程中
    bool eventHandling_; //是否在loop()中处于处理每个事件的阶段
    
public:
    Epoll poller_;
private:
    int wakeupFd_;
    bool quit_;
    mutable MutexLock mutex_; //mutable: 在const成员函数也可以修改
    std::vector<Functor> pendingFunctors_;//queueInLoop作用的位置，实际上是一个待调用函数的队列？
    bool callingPendingFunctors_; //是否处于调用pending Functor调用过程中
    const pid_t threadId_;
    std::shared_ptr<Channel> pWakeupChannel;
    std::shared_ptr<TimerQueue> timerQueue_;
    
    
    void handleRead(); //处理"被唤醒"事件
    void handleConn(); //更新eventFlag
    void doPendingFunctors(); //将pendingFunctors_中的函数逐个取出完成
public:
    void mainReactorHandleRead(); //主Reactor接收到读事件，也就是有新的连接被建立

    void addChannel(std::shared_ptr<Channel> channel, int timeout);

    void assertInReactorThread();//

    TimerId runAt(Timestamp time, Timer::TimerCallback cb){
        return timerQueue_->addTimer(std::move(cb), time, 0.0);
    }

    TimerId runAfter(double delayInSeconds, Timer::TimerCallback cb){
        Timestamp time(addTime(Timestamp::now(), delayInSeconds));
        return runAt(time, std::move(cb));
    }

    
};
