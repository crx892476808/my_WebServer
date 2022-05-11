/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-10 14:54:03
 * @LastEditTime: 2022-05-11 19:30:32
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#pragma once

#include<pthread.h>
#include<vector>
#include<memory>
#include"Reactor.h"
#include"Thread.h"
#include"mylib/Noncopyable.h"

class ThreadPool: Noncopyable{
public:
    ThreadPool(Reactor *mainReactor, int numThreads);
    ~ThreadPool();
    void start();
    Reactor *getNextReactor(); // 原项目的实现是RoundRobin，可以考虑改成其他的实现形式
private:
    Reactor *mainReactor_;
    int numThreads_;
    bool started_;
    int next_;
    std::vector<std::shared_ptr<Thread>> threads_;
    std::vector<Reactor*> reactors_;
};