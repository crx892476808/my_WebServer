/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-17 17:12:51
 * @LastEditTime: 2022-05-17 19:49:07
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */

#pragma once

#include "Channel.h"
#include "Reactor.h"
#include "ThreadPool.h"
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
class Server{ 
public:
    Server(Reactor* mainReactor, ThreadPool* threadPool): mainReactor_(mainReactor), threadPool_(threadPool){
        int acceptFd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(8000);
        addr.sin_addr.s_addr = INADDR_ANY;
        bind(acceptFd, (struct sockaddr*)&addr,  sizeof(struct sockaddr_in));
        listen(acceptFd, 5);
        
        acceptChannel_ = std::make_shared<Channel>(mainReactor, acceptFd);
        acceptChannel_->eventFlag_ = EPOLLIN | EPOLLET;
        acceptChannel_->setReadHandler(std::bind(&Server::mainReactorHandleRead, this));
        acceptChannel_->setConnHandler(std::bind(&Server::mainReactorHandleConn, this));
        mainReactor_->poller_.epoll_add(acceptChannel_, 0);
    }
    std::shared_ptr<Channel> acceptChannel_; //专用于接收新连接的channel
    Reactor* mainReactor_;
    ThreadPool *threadPool_; // 所关联的唯一线程库
    void mainReactorHandleRead();//由mainReactor处理新的连接
    void mainReactorHandleConn();
};