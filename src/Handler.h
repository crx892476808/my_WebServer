/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-15 10:52:47
 * @LastEditTime: 2022-05-15 13:58:12
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#pragma once

#include<functional>
#include<memory>
#include<sys/epoll.h>
#include"Event.h"
class Reactor;
class HttpData;

class Handler{
private:
    typedef std::function<void()> CallBack;
    Reactor *reactor_;


    // 方便找到上层持有该Channel的对象
    std::weak_ptr<HttpData> holder_;

    
    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;

public:
    Handler(Reactor* reactor);
    Handler(Reactor* reactor, int fd);
    void handleEvents(Event *event){
        if((event->returnEventFlag_ & EPOLLHUP) && !(event->returnEventFlag_ & EPOLLIN)){ //EPOLLHUP表示文件被挂断。EPOLLIN表示有新数据到达，socket可读，或对端写通道关闭
        }
        if(event->returnEventFlag_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){ //EPOLLRDHUP表示对端关闭连接或者关闭写入半连接。EPOLLPRI : 文件有紧急数据可读
            if (readHandler_) {
                readHandler_();
            }
        }
        if(event->returnEventFlag_ & EPOLLOUT){
            if (writeHandler_){
                writeHandler_();
            }
        }
        if(connHandler_){
            connHandler_();
        }
    }
};