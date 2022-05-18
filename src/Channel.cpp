/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-16 23:41:41
 * @LastEditTime: 2022-05-17 17:16:07
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */

#include "Channel.h"
#include <iostream>



Channel::Channel(Reactor* reactor, int fd):  
    fd_(fd), eventFlag_(0), returnEventFlag_(0), 
    lastEventFlag_(0), reactor_(reactor) {
}

