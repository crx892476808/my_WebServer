// @Author Armin Jager
// @Email xxbbb@vip.qq.com

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <memory>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "Epoll.h"
#include "Channel.h"
#include "Server.h"
#include "Reactor.h"
#include "ThreadPool.h"
#include "unitTest/timer_unitTest.hh"

int main() {
    TestTimer();

    // Reactor mainReactor;
    // ThreadPool threadPool(&mainReactor, 2);
    // threadPool.start();
    // //TODO : MainReactor打开socket，监听指定端口，发生事件后，将相应的连接socket（channel）交给某个subReactor

    
    // Server server(&mainReactor, &threadPool);
    // while(true){
    //     mainReactor.loop();
    //     std::cout << "break from mainReactor.loop()" << std::endl;
    //     // socklen_t clientAddrLen = sizeof(sockaddr_in);
    //     // int connFd = accept(acceptFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    //     // char clientIP[INET_ADDRSTRLEN] = "";
    //     //         inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    //     // std::cout << "...connect " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl; //`sin`的意思是socket_internet
    // }
    // return 0;
}


