/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-10 12:07:21
 * @LastEditTime: 2022-05-11 19:29:31
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#pragma once
#include"Noncopyable.h"
#include"MutexLock.h"
#include"Condition.h"

// CountDownLatch的主要作用是确保Thread中传进去的func真的启动了以后
// 外层的start才返回
// 维护一个倒数计数器，计数器为0视为条件变量为真
class CountDownLatch: Noncopyable{
public:
    explicit CountDownLatch(int);
    void wait();
    void countDown();
private:
    int count_;
    MutexLock mutex_;
    Condition condition_;
};
