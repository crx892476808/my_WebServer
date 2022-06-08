/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-16 23:41:41
 * @LastEditTime: 2022-06-08 17:52:46
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */

#include "Channel.h"
#include "Reactor.h"
#include <iostream>



Channel::Channel(Reactor* reactor, int fd):  
    fd_(fd), eventFlag_(0), returnEventFlag_(0), 
    lastEventFlag_(0), reactor_(reactor) {
}

void Channel::enableReading(){
    this->eventFlag_ = EPOLLIN | EPOLLPRI | EPOLLOUT;
    reactor_->addChannel(this, 0);
}