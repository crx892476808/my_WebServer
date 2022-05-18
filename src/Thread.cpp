/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-10 11:59:10
 * @LastEditTime: 2022-05-17 14:05:01
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#include "Thread.h"
#include <assert.h>
#include <errno.h>
#include <linux/unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include "mylib/CurrentThread.h"
#include <functional>
#include <iostream>
using namespace std;

// Thread control
namespace CurrentThread {
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char* t_threadName = "default";
}

// 为了在线程中保留name, tid这些数据
struct ThreadData{
    Thread::ThreadFunc func_;
    std::string name_;
    pid_t *tid_;
    CountDownLatch *latch_;
    

    ThreadData(const Thread::ThreadFunc& func, const std::string &name, pid_t *tid, CountDownLatch *latch): 
        func_(func), name_(name), tid_(tid), latch_(latch){}

    void runInThread(){
        *tid_ = CurrentThread::tid();
        tid_ = NULL;
        latch_->countDown();
        latch_ = NULL;

        CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
        prctl(PR_SET_NAME, CurrentThread::t_threadName); // 指定进程的名字
        func_();
        CurrentThread::t_threadName = "finished";
    }
};

void *startThread(void *obj){
    ThreadData *data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

Thread::Thread( const string& n)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(0),
      func_(std::bind(&Thread::threadFunc,this)),
      name_(n),
      latch_(1),

      reactor_(nullptr),
      exiting_(false),
      mutex_(),
      cond_(mutex_){
          setDefaultName(); //name_ = "Thread"
}

Thread::~Thread(){
    exiting_ = true;
    if(started_ && !joined_) pthread_detach(pthreadId_);
}

void Thread::setDefaultName() {
  if (name_.empty()) {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread");//snprintf()函数用于将格式化的数据写入字符串, buf为要写入的字符串，sizeof是要写入字符的最大个数，"Thread"是要写入的内容
    name_ = buf;
  }
}

//创建线程并开始执行新线程，父线程等待在CountDownLatch上
void Thread::start(){
    assert(!started_);
    started_ = true; // 已经开始
    ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);
    if(pthread_create(&pthreadId_, NULL, &startThread, data)){//unsigned long, pthread_attr_t*, void *(*start_routine) (void *), void *arg(data表示传给startThread的参数)
        //非0返回值表示创建线程失败
        started_ = false;
        delete data;
    }
    else{ //为0表示创建成功，并且子线程开始执行 (子线程从start_routine函数的起点地址开始运行)
        latch_.wait(); //确保func_确实执行后，start()才返回
        assert(tid_ > 0);  //表明子线程已经成功运行起来(否则不会改变tid_)
    }
}

int Thread::join(){
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL); 
    //如果想获取某个线程执行结束时返回的数据，可以调用 pthread_join() 函数来实现。
    //参数1用于指定接收哪个线程的返回值；参数2表示接收到的返回值
    //pthread_join() 函数会一直阻塞调用它的线程，直至目标线程执行结束（接收到目标线程的返回值），阻塞状态才会解除。
    //如果 pthread_join() 函数成功等到了目标线程执行结束（成功获取到目标线程的返回值），返回值为数字 0；反之执行失败
}


// Reactor related
Reactor* Thread::startLoop(){
    assert(!started_);
    start();
    {
        MutexLockGuard lock(mutex_);
        //一直等到threadFun在Thread里真正跑起来
        while(reactor_ == nullptr) cond_.wait(); // 这个cond_用于 reactor_ == nullptr 的判断
    }
    return reactor_;
}

void Thread::threadFunc(){
    Reactor reactor;
    {
        MutexLockGuard lock(mutex_);
        reactor_ = &reactor;
        cond_.notify(); //此时可以从startLoop()退出，表明子线程已经可以正常工作
    }
    reactor.loop();
    reactor_ = nullptr;
}

pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

void CurrentThread::cacheTid() {
  if (t_cachedTid == 0) {
    t_cachedTid = gettid();
    t_tidStringLength =
        snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}