/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-16 23:23:43
 * @LastEditTime: 2022-06-01 11:46:30
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
# pragma once

#include "Channel.h"
#include <memory>
class HttpHandler{ //主要是为了实现更简单地绑定handleRead等函数
public:
    HttpHandler(std::shared_ptr<Channel> channel): channel_(channel),  reactor_(channel->reactor_), shouldBeClose_(false){}
    std::shared_ptr<Channel> channel_; //该HttpHandler所对应的Channel
    Reactor *reactor_;
    void HttpReadHandle();
    void HttpConnHandle();

    void bindToChannel();
    bool shouldBeClose_;
private:
    void parseHttpRequest();
    std::string rawHttpRequest_;
    std::string httpMethod_;
    std::string url_;
    std::string httpVersion_;

};