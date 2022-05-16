/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-11 09:02:40
 * @LastEditTime: 2022-05-16 23:12:13
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#include"Reactor.h"
#include"mylib/CurrentThread.h"
#include<iostream>

Reactor::Reactor(): looping_(false), poller_(), wakeupFd_(0), quit_(false),
eventHandling_(false),callingPendingFunctors_(false), 
threadId_(CurrentThread::tid())
{

};

void Reactor::quit(){

}
void Reactor::loop(){
    std::cout << "in Reactor::loop()" << std::endl;
    while(true);
    
}
