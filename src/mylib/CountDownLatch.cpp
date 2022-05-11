/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-10 12:55:46
 * @LastEditTime: 2022-05-10 13:45:55
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */

#include"CountDownLatch.h"

CountDownLatch::CountDownLatch(int count): count_(count), mutex_(), condition_(mutex_){}

void CountDownLatch::wait(){
    MutexLockGuard lock(mutex_); //析构函数中自动解锁
    count_++;
    condition_.wait();
}

void CountDownLatch::countDown(){
    MutexLockGuard lock(mutex_); //析构函数中自动解锁
    count_--;
    if(count_ == 0){
        condition_.notifyAll();
    }
}
