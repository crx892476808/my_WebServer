/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-15 13:11:31
 * @LastEditTime: 2022-05-15 14:31:05
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */

#pragma once
#include<functional>
class Event{
public:
    int fd_; // 事件对应的文件描述符
    uint32_t eventFlag_; //原项目的events_, 对应epoll_event::events
    uint32_t returnEventFlag_; //原项目的Channel::revents_
    uint32_t lastEventFlag_;
    
};
