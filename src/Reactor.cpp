/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-11 09:02:40
 * @LastEditTime: 2022-05-24 16:10:49
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#include"Reactor.h"
#include"mylib/CurrentThread.h"
#include"mylib/Util.h"
#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>
#include <assert.h>
#include <functional>

int createEventFd(){
    int eventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(eventFd < 0){
        abort();
    }
    return eventFd;
}

Reactor::Reactor(): looping_(false),  eventHandling_(false), poller_(),
     quit_(false), callingPendingFunctors_(false), 
    threadId_(CurrentThread::tid())
{
    wakeupFd_ = createEventFd();
    std::cout << "wakeupFd_ in thread" << CurrentThread::tid() << " is" << wakeupFd_ << std::endl;  
    pWakeupChannel = std::make_shared<Channel>(this, wakeupFd_);
    //pWakeupChannel = 
    pWakeupChannel->setReadHandler(std::bind(&Reactor::handleRead, this)); //主Reactor所需要使用的Handler
    pWakeupChannel->setConnHandler(std::bind(&Reactor::handleConn, this)); 
    pWakeupChannel->eventFlag_ = EPOLLIN | EPOLLET;
    poller_.epoll_add(pWakeupChannel, 0);
    std::cout << CurrentThread::name() << " in thread " << CurrentThread::tid() << std::endl;
};

Reactor::~Reactor(){
    close(wakeupFd_);
}

void Reactor::quit(){
    quit_ = true;
    if(threadId_ != CurrentThread::tid()){ //WHY??
        ;//wakeup();
    }
}
void Reactor::loop(){
    std::cout << "in Reactor::loop()" << std::endl;
    assert(!looping_);
    assert(CurrentThread::tid() == threadId_);
    looping_ = true;
    quit_ = false;
    std::vector<std::shared_ptr<Channel>> ret; 
    while(!quit_){
        ret.clear();
        ret = poller_.poll();
        eventHandling_ = true;
        for(std::shared_ptr<Channel> c : ret) 
            c->handleEvent();
        eventHandling_ = false;
        doPendingFunctors();
    }
    looping_ = false;
    
    
}

void Reactor::runInLoop(Functor &&f){
    if(threadId_ == CurrentThread::tid()){
        f();
    }
    else{
        queueInLoop(std::move(f));
    }
}

void Reactor::queueInLoop(Functor &&f){
    {
        MutexLockGuard guard(mutex_);
        pendingFunctors_.push_back(std::move(f));
    }
    if(threadId_ != CurrentThread::tid()){
        wakeup();
    }
}

void Reactor::wakeup(){
    uint64_t wakeupSignal = 1;
    ssize_t n = writen(wakeupFd_, (char*)&wakeupSignal, sizeof wakeupSignal);
    if(n != sizeof wakeupSignal){
        std::cout << "wakeup() writes" << n << "bytes instead of " << sizeof wakeupSignal << std::endl;
    }
}

void Reactor::handleRead(){
    std::cout << "Reactor::handleRead()" << std::endl;
    uint64_t wakeupSignal = 0;
    ssize_t n = readn(wakeupFd_, (char*)&wakeupSignal, sizeof wakeupSignal);
    if(n != sizeof wakeupSignal){
        std::cout << "wakeup() reads" << n << "bytes instead of " << sizeof wakeupSignal << std::endl;
    }
    pWakeupChannel->eventFlag_ = EPOLLIN | EPOLLET; //WHY：为什么这里要重设eventFlag？
    // 猜想：尽管这里修改的是Channel的eventFlag，但可以通过epoll_mod传递给fd，所以应该就是设置需要监听的事件类型
}


void Reactor::handleConn(){
    std::cout << "Reactor::handleConn()" << std::endl;
    poller_.epoll_modify(pWakeupChannel, 0); // 更新wakeupChannel的eventFlag
}

void Reactor::doPendingFunctors(){
    std::cout << "Reactor::doPendingFunctors()" << std::endl;
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        MutexLockGuard guard(mutex_);
        functors.swap(pendingFunctors_); //swao data with another vector
    }
    std::cout << "Reactor::doPendingFunctors() -- 2" << std::endl;
    for(size_t i = 0;i < functors.size();i++) functors[i]();
    std::cout << "Reactor::doPendingFunctors() -- 3" << std::endl;
    callingPendingFunctors_ = false;
}

void Reactor::addChannel(std::shared_ptr<Channel> channel, int timeout){
    poller_.epoll_add(channel,0);
}