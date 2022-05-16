/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-15 13:11:31
 * @LastEditTime: 2022-05-16 23:23:28
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */

#pragma once
#include <functional>
#include <sys/epoll.h>
class Reactor;

class Channel{
public:
    int fd_; // 事件对应的文件描述符
    uint32_t eventFlag_; //原项目的events_, 对应epoll_event::events
    uint32_t returnEventFlag_; //原项目的Channel::revents_
    uint32_t lastEventFlag_; //上一次的eventFlag

private:
    Reactor *reactor_;
    typedef std::function<void()> CallBack;
    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;
    void handleEvent(){
        if( (returnEventFlag_ & EPOLLHUP) && !(returnEventFlag_ & EPOLLIN)){ //EPOLLHUP表示文件被挂断。EPOLLIN表示有新数据到达，socket可读，或对端写通道关闭
            //TODO: handle error
        }
        if(returnEventFlag_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){ //EPOLLRDHUP表示对端关闭连接或者关闭写入半连接。EPOLLPRI : 文件有紧急数据可读
            if (readHandler_) {
                readHandler_();
            }
        }
        if(returnEventFlag_ & EPOLLOUT){
            if (writeHandler_){
                writeHandler_();
            }
        }
        if(connHandler_){
            connHandler_();
        }
    }
};
