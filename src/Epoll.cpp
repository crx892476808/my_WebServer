/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-15 14:15:47
 * @LastEditTime: 2022-05-15 17:20:54
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */

#include"Epoll.h"

#include <sys/epoll.h>
#include <iostream>
#include <assert.h>

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

Epoll::Epoll(): epollEvents_(EVENTSNUM){
    epollFd_ = epoll_create1(EPOLL_CLOEXEC);//Set the close-on-exec (FD_CLOEXEC) flag on the new file descriptor
//如果设置为EPOLL_CLOEXEC，那么由当前进程fork出来的任何子进程，其都会关闭其父进程的epoll实例所指向的文件描述符，也就是说子进程没有访问父进程epoll实例的权限。
    assert(epollFd_ > 0);
}
Epoll::~Epoll(){
    
}

void Epoll::epoll_add(std::shared_ptr<Event> event, int timeout){
    int eventFd = event->fd_;
    //TODO: Add timer 
    struct epoll_event ep_event; //The events member is a bit mask composed by ORing together zero or more of the following available event types
    ep_event.data.fd = eventFd;  
    ep_event.events = event->eventFlag_;
    fd2event[eventFd] = event;
    std::cout << "Epoll::epoll_add() : eventFD=" << eventFd << " " << "epollFd=" << epollFd_ <<  std::endl;
    if(epoll_ctl(epollFd_, EPOLL_CTL_ADD, eventFd, &ep_event) < 0){
        std::cout << "Error occur in epoll_add" << std::endl;
    }
}

std::vector<std::shared_ptr<Event>> Epoll::poll(){
    while(true){
        int event_count = epoll_wait(epollFd_, &(*epollEvents_.begin()), epollEvents_.size(), EPOLLWAIT_TIME);//refer to `man 2 epoll_wait`
        if(event_count < 0) std::cout << "Error in Epoll::poll()" << std::endl;
        else if(event_count == 0) std::cout << "No available active fd" << std::endl;
        else std::cout << "available active fd" << std::endl;
        std::vector<std::shared_ptr<Event>> events = getEvents(event_count);
        if(events.size() > 0) return events;
    }
}

std::vector<std::shared_ptr<Event>> Epoll::getEvents(int event_count){
    std::vector<std::shared_ptr<Event>> events;
    for(int i = 0;i != event_count;i++){
        std::shared_ptr<Event> eventPtr = fd2event[epollEvents_[i].data.fd];
        if(eventPtr){
            eventPtr->returnEventFlag_ = epollEvents_[i].events;
            eventPtr->eventFlag_ = 0;
            events.push_back(eventPtr);
        }
    }
    return events;
}