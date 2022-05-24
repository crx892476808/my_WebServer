/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-15 13:11:31
 * @LastEditTime: 2022-05-24 12:02:25
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */

#pragma once
#include <functional>
#include <sys/epoll.h>
#include <memory>
class Reactor;
class HttpHandler;
class Channel{
private:
    typedef std::function<void()> CallBack;
public:
    Channel(Reactor*,int);
    
    int fd_; // 信道对应的文件描述符
    uint32_t eventFlag_; //原项目的events_, 对应epoll_event::events
    uint32_t returnEventFlag_; //原项目的Channel::revents_
    uint32_t lastEventFlag_; //上一次的eventFlag
    Reactor *reactor_;
    
    void setReadHandler(CallBack readHandler){readHandler_ = readHandler;}
    void setWriteHandler(CallBack writeHandler){writeHandler_ = writeHandler;}
    void setErrorHandler(CallBack errorHandler){errorHandler_ = errorHandler;}
    void setConnHandler(CallBack connHandler){connHandler_ = connHandler;}

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

    void setHttpHandler(std::shared_ptr<HttpHandler> httpHandler){httpHandler_ = httpHandler;}

private:
    
    
    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;

    std::shared_ptr<HttpHandler> httpHandler_;


};
