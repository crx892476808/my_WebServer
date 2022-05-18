/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-16 23:23:43
 * @LastEditTime: 2022-05-18 19:36:45
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
# pragma once

#include "Channel.h"
#include <memory>
class HttpHandler{ //主要是为了实现更简单地绑定handleRead等函数
public:
    HttpHandler(std::shared_ptr<Channel> channel): channel_(channel), shouldBeClose_(false){}
    std::shared_ptr<Channel> channel_; //该HttpChannel所对应的Channel
    void HttpReadHandle();
    void HttpConnHandle();
    bool shouldBeClose_;
};