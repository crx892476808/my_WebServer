/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-17 19:28:29
 * @LastEditTime: 2022-05-24 16:17:38
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#include "HttpHandler.h"
#include "Reactor.h"
#include "mylib/Util.h"
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>

void HttpHandler::parseHttpRequest(){
    //rawHttpRequest_
    std::istringstream sstr(rawHttpRequest_);
    sstr >> httpMethod_;
    sstr >> url_;
    sstr >> httpVersion_; 
}

void HttpHandler::HttpReadHandle(){
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
    rawHttpRequest_ = str;
    // parse Http Request
    parseHttpRequest();
    std::string response_str;
    // 静态路由方式，确定需要返回的内容
    if(url_ == "/"){
        std::cout << "url == /" << std::endl;
        response_str = "HTTP/1.0 200 OK\r\nContent-type: text/plain\r\nContent-length: 7\r\n\r\nWelcome";
        //response_str = "Welcome";
        std::cout << "To write bytes: " << response_str.size() << std::endl;
        //response_str += std::string(512,'a');
    }
    // write reply to connFd
    writen(connFd, response_str);
    //close(connFd);
    
    
}

void HttpHandler::HttpConnHandle(){
    std::cout << "*** HttpChannel::HttpConnHandle() ***" << std::endl;
    if(shouldBeClose_){
        close(channel_->fd_);
        channel_->reactor_->poller_.epoll_delete(channel_,0);
        return;
    }
    channel_->eventFlag_ = EPOLLIN | EPOLLET;
    channel_->reactor_->poller_.epoll_modify(channel_, 0);
}

void HttpHandler::bindToChannel(){
    if(reactor_ == nullptr){
        std::cout << "reactor_ in HttpHandler::bindToChannel is nullptr" << std::endl;
    }
    reactor_->poller_.epoll_add(channel_, 0);
}