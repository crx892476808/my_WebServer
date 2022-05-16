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

int main() {
    std::cout << "This is server" << std::endl;
    // socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        std::cout << "Error: socket" << std::endl;
        return 0;
    }
    // bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cout << "Error: bind" << std::endl;
        return 0;
    }
    // listen
    if(listen(listenfd, 5) == -1) {
        std::cout << "Error: listen" << std::endl;
        return 0;
    }
    // accept
    int conn;
    char clientIP[INET_ADDRSTRLEN] = "";
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    Epoll epoll;

    
    while (true) {
        std::cout << "...listening" << std::endl;
        conn = accept(listenfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::cout << "...connect " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
        
        std::cout << "new fd = " << conn << std::endl;
        std::shared_ptr<Channel> event(new Channel);
        event->fd_ = conn;
        event->eventFlag_ = EPOLLIN | EPOLLET;
        epoll.epoll_add(event, 0);
        while(true){
            std::vector<std::shared_ptr<Channel>> ret = epoll.poll();
            if(ret.size() > 0){
                std::cout << "recive from client!" << std::endl;
            }
            bool should_exit = false;
            for(auto e: ret){
                char buf[255];
                memset(buf, 0, sizeof(buf));
                int len = read(e->fd_, buf, sizeof(buf));
                buf[len] = '\0';
                if (strcmp(buf, "exit") == 0) {
                    std::cout << "...disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
                    should_exit = true;
                }
                std::cout << buf << std::endl;
                send(e->fd_, buf, len, 0);
            }
            if(should_exit)
                break;

        }

        // if (conn < 0) {
        //     std::cout << "Error: accept" << std::endl;
        //     continue;
        // }
        // inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        // std::cout << "...connect " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

        // char buf[255];
        // while (true) {
        //     memset(buf, 0, sizeof(buf));
        //     int len = recv(conn, buf, sizeof(buf), 0);
        //     buf[len] = '\0';
        //     if (strcmp(buf, "exit") == 0) {
        //         std::cout << "...disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
        //         break;
        //     }
        //     std::cout << buf << std::endl;
        //     send(conn, buf, len, 0);
        // }
        
        close(conn);
    }
    close(listenfd);
    return 0;
}



// #include <string>
// #include <iostream>
// #include <memory>

// #include "Server.h"
// #include "ThreadPool.h"
// int main(int argc, char *argv[])
// {
//     // std::string logPath = "./WebServer.log";
//     // Reactor *mainReactor = new Reactor();
//     // ThreadPool threadPool(mainReactor, 3);
//     // threadPool.start();
//     std::shared_ptr<int> p(new int(3));
//     std::shared_ptr<int> ap[100];
//     ap[0] = p;
//     {
//         std::shared_ptr<int> p2(new int(5));
//         ap[1] = p2;
//     }
//     std::cout << *ap[1] << std::endl;

// }