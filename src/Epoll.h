/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-15 14:02:07
 * @LastEditTime: 2022-06-08 18:11:05
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#pragma once

#include"Channel.h"

#include<memory>
#include<vector>
#include<unordered_map>
#include <sys/epoll.h>

class Epoll{ //封装 sys/epoll.h
public:
    Epoll();
    ~Epoll();
    void epoll_add(std::shared_ptr<Channel> event, int timeout);
    void epoll_add(Channel* event, int timeout);
    void epoll_modify(std::shared_ptr<Channel> channel, int timeout);
    void epoll_delete(std::shared_ptr<Channel> channel, int timeout);
    std::vector<std::shared_ptr<Channel>> poll();
private:
    int epollFd_;
    std::vector<struct epoll_event> epollEvents_;
    std::vector<std::shared_ptr<Channel>> getEvents(int eventNum);
    std::unordered_map<int, std::shared_ptr<Channel>> fd2channel;
    std::unordered_map<int, Channel*> fd2channelStar;
};
