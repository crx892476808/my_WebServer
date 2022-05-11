/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-10 07:24:15
 * @LastEditTime: 2022-05-10 07:38:00
 * @LastEditors: Armin Jager
 * @Description: 
 */
#pragma once

class Noncopyable
{
protected:
    Noncopyable(){}
    ~Noncopyable(){}
    Noncopyable(const Noncopyable &) = delete;
    Noncopyable &operator=(const Noncopyable &) = delete;
};