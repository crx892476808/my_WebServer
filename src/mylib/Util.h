/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-17 08:22:56
 * @LastEditTime: 2022-05-18 14:50:39
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#pragma once

#include<cstdlib>
#include<string>

//fd read/write
ssize_t  writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &str);
ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &str, bool &isReadZero);

//fd management
int setSocketNonBlocking(int fd);