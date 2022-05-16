/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-15 14:02:07
 * @LastEditTime: 2022-05-16 23:18:32
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
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
    std::vector<std::shared_ptr<Channel>> poll();
private:
    int epollFd_;
    std::vector<struct epoll_event> epollEvents_;
    std::vector<std::shared_ptr<Channel>> getEvents(int eventNum);
    std::unordered_map<int, std::shared_ptr<Channel>> fd2event;
};
