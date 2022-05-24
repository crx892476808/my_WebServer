/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-17 17:14:46
 * @LastEditTime: 2022-05-24 17:51:53
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */

#include"Server.h"
#include"Reactor.h"
#include"HttpHandler.h"
#include"mylib/Util.h"

#include<iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
void Server::mainReactorHandleRead(){
    std::cout << "*** Server::mainReactorHandleRead() ***" << std::endl;
    int acceptFd = acceptChannel_->fd_;
    socklen_t clientAddrLen = sizeof(sockaddr_in);
    struct sockaddr_in clientAddr;
    int connFd = accept(acceptFd, (struct sockaddr*)&clientAddr, &clientAddrLen); //接收新的TCP连接
    char clientIP[INET_ADDRSTRLEN] = "";
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    std::cout << "...connect " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl; //`sin`的意思是socket_internet

    //将connFd分给某个SubReactor的poller
    Reactor* selectedSubReactor = threadPool_->getNextReactor();
    //将fd设为非阻塞式，不延迟
    setSocketNonBlocking(connFd);
    setSocketNodelay(connFd);
    
    std::shared_ptr<Channel> httpChannel(new Channel(selectedSubReactor,connFd));
    std::shared_ptr<HttpHandler> httpHandler = std::make_shared<HttpHandler>(httpChannel);
    httpChannel->setHttpHandler(httpHandler);
    httpChannel->setReadHandler(bind(&HttpHandler::HttpReadHandle, httpHandler));
    httpChannel->setConnHandler(bind(&HttpHandler::HttpConnHandle, httpHandler));
    httpChannel->eventFlag_ = EPOLLIN | EPOLLET;
    //selectedSubReactor->poller_.epoll_add(httpChannel,0);
    selectedSubReactor->queueInLoop(std::bind(&HttpHandler::bindToChannel,httpHandler)); //不直接进行epoll_add，是希望这个操作放到subReactor线程做，避免线程安全问题
    //通知分配新连接这件事情发生了(修改后，这一步已经放到了runInLoop一并完成)
    //selectedSubReactor->wakeup();    
}

void Server::mainReactorHandleConn(){
    std::cout << "*** Server::mainReactorHandleConn() ***" << std::endl;
    acceptChannel_->eventFlag_ = EPOLLIN | EPOLLET;
    mainReactor_->poller_.epoll_modify(acceptChannel_, 0);
}