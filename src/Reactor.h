/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-11 08:41:15
 * @LastEditTime: 2022-05-15 15:46:51
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#pragma once
#include"mylib/MutexLock.h"
#include<functional>
#include<vector>
class Reactor{
public:
    typedef std::function<void()> Functor;
    //Reactor();
    //~Reactor();
    void quit();
    void loop();

private:
    bool looping_;

    int wakeupFd_;
    bool quit_;
    bool eventHandling_;
    mutable MutexLock mutex_; //mutable: 在const成员函数也可以修改
    std::vector<Functor> pendingFunctors_;
    bool callingPendingFunctors_;
    const pid_t threadId_ = 0;
    
    

};
