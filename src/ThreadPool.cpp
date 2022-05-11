/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-11 12:38:21
 * @LastEditTime: 2022-05-11 15:34:53
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */

#include"ThreadPool.h"
#include"assert.h"
#include<iostream>
ThreadPool::ThreadPool(Reactor *mainReactor, int numThreads): mainReactor_(mainReactor), numThreads_(numThreads), started_(false), next_(0){
    if(numThreads_ <= 0){
        abort();
    }
}

ThreadPool::~ThreadPool(){
}

void ThreadPool::start(){
    started_ = true;
    for(int i = 0;i < numThreads_;i++){
        std::shared_ptr<Thread> thread ( new Thread("SubReactor")) ;
        Reactor* subreactor = thread->startLoop();
        threads_.push_back(thread);
        reactors_.push_back(subreactor);
    }
}

Reactor* ThreadPool::getNextReactor(){
    assert(started_);
    next_ = (next_ + 1) % numThreads_;
    Reactor* reactor =  reactors_[next_];
    return reactor;
}