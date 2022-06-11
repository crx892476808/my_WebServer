#include"sys/timerfd.h"
#include<unistd.h>
#include<iostream>
#include<functional>
#include<cstring>

#include"TimerQueue.h"
#include"../Reactor.h"


static int createTimerfd(){
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0){
        std::cout << "Failed in timerfd_create" << std::endl;
    }
    return timerfd;
}

static void readTimerfd(int timerfd, Timestamp now){
    uint64_t timeoutTimes; // 记录超时次数
    ssize_t n = read(timerfd, &timeoutTimes, sizeof timeoutTimes);
    if(n != sizeof timeoutTimes){
        std::cout << "TimerQueue::handleRead() reads " << n << "bytes instead of 8" << std::endl;
    }
}

static timespec howMuchTimeFromNow(Timestamp when){
    int64_t microSeconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if(microSeconds < 100){
        microSeconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microSeconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microSeconds % Timestamp::kMicroSecondsPerSecond) * 1000); // 纳秒
    return ts;
}

static void resetTimerfd(int timerfd, Timestamp expiration){
    std::cout << "*** resetTimerfd ***" << std::endl;
    // wake up loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof newValue);
    memset(&oldValue, 0, sizeof oldValue);

    newValue.it_value = howMuchTimeFromNow(expiration);
    std::cout << "newValue.it_value " << newValue.it_value.tv_sec << std::endl;
    int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue); //以newValue设置下一次的超时时间，将之前的超时信息放在oldValue
    if(ret){
        std::cout << "err in resetTimerfd()" << std::endl;
    }
}

TimerQueue::TimerQueue(Reactor* reactor):
    reactor_(reactor), 
    timerfd_(createTimerfd()),
    timerfdChannel_(new Channel(reactor, timerfd_)),
    timers_()
{
    timerfdChannel_->setReadHandler(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_->eventFlag_ = EPOLLIN | EPOLLPRI;
    reactor_->addChannel(timerfdChannel_, 0);
}

TimerQueue::~TimerQueue(){
    close(timerfd_);
    // for (const TimerEntry& timer: timers_){
    //     delete timer.second;
    // }
    //timerfdChannel_.reactor_->poller_.
}

std::vector<TimerQueue::TimerEntry> TimerQueue::getExpired(Timestamp now){
    std::vector<TimerEntry> expired;
    TimerEntry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX)); // 作为哨兵，去找到now之前过期的定时器
    TimerList::iterator end = timers_.lower_bound(sentry); // 返回第一个大于哨兵给定时间的数
    assert(end == timers_.end() || now < end -> first);
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);
    return expired;
}

void TimerQueue::handleRead(){
    reactor_->assertInReactorThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now); // 先把timerfd给读出

    std::vector<TimerEntry> expired = getExpired(now); // 确认哪些计时器发生了超时
    callingExpiredTimers_ = true; 
    for(const TimerEntry& it: expired){
        it.second->run(); //进行回调函数的调用
    }
    callingExpiredTimers_ = false;
}

TimerId TimerQueue::addTimer(Timer::TimerCallback cb, Timestamp when, double interval){
    Timer *timer = new Timer(std::move(cb), when, interval);
    auto f = std::bind(&TimerQueue::addTimerInLoop, this, timer);
    reactor_->runInLoop(f);
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancelTimer(TimerId timerId){

}

void TimerQueue::cancelTimerInLoop(Timer* timer){
    reactor_->assertInReactorThread();
}

void TimerQueue::addTimerInLoop(Timer* timer){
    std::cout << "*** TimerQueue::addTimerInLoop ***" <<std::endl;
    reactor_->assertInReactorThread();
    bool earliestChanged = insert(timer);
    if(earliestChanged){ // 如果出现更早到期的定时器，需要重置定时器fd
        std::cout << "in earliestChanged" << std::endl;
        resetTimerfd(timerfd_, timer->expiration());
    }

}

bool TimerQueue::insert(Timer *timer){
    reactor_->assertInReactorThread();
    bool earliestChanged = false; //最早到期的计时器是否发生变化
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if(it == timers_.end() || when < it->first){
        earliestChanged = true;
    }
    {
        std::pair<TimerList::iterator, bool> result = timers_.insert(TimerEntry(when, timer));
        assert(result.second);
    }
    return earliestChanged;
}