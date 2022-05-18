/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-17 19:28:29
 * @LastEditTime: 2022-05-18 19:38:58
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#include "HttpHandler.h"
#include "Reactor.h"
#include "mylib/Util.h"
#include <iostream>
#include <string>
void HttpHandler::HttpReadHandle(){
    // do something with connFd
    // read from connFd
    std::cout << "*** HttpChannel::HttpReadHandle() ***" << std::endl;
    int connFd = channel_->fd_;
    std::string str;
    bool isReadZero = false;
    readn(connFd, str, isReadZero);
    if(isReadZero){ //Connection Close
        std::cout << "connection close by the client" << std::endl;
        shouldBeClose_ = true;
        return ;
    }
    std::cout << "read result: " << str << std::endl;
    // write reply to connFd
    writen(connFd, str);
}

void HttpHandler::HttpConnHandle(){
    std::cout << "*** HttpChannel::HttpConnHandle() ***" << std::endl;
    if(shouldBeClose_){
        channel_->reactor_->poller_.epoll_delete(channel_,0);
        return;
    }
    channel_->eventFlag_ = EPOLLIN | EPOLLET;
    channel_->reactor_->poller_.epoll_modify(channel_, 0);
}