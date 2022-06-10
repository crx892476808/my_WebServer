#pragma once
#include<set>
#include<vector>
#include<utility>
#include<memory>
#include<assert.h>
#include"Noncopyable.h"
#include"Timestamp.h"
#include"Timer.h"
#include"TimerId.h"
#include"../Channel.h"


class Reactor;

//using std::set; using std::pair; using std::share


class TimerQueue: Noncopyable{
public:
    explicit TimerQueue(Reactor* reactor);
    ~TimerQueue();

    //Schedules the callback to be run at given time,
    //must be thread safe, usually be called from other threads
    //repeats if @c interval > 0.0
    TimerId addTimer(Timer::TimerCallback cb, Timestamp when, double interval); // when表示调用回调函数cb的时间

private:
    typedef std::pair<Timestamp, Timer*> TimerEntry;
    typedef std::set<TimerEntry> TimerList;


    Reactor* reactor_;
    const int timerfd_;
    std::shared_ptr<Channel> timerfdChannel_;
    // Timer list sorted by expiration
    TimerList timers_;
    bool callingExpiredTimers_;

    void handleRead();
    std::vector<TimerEntry> getExpired(Timestamp now);
    void addTimerInLoop(Timer* timer);
    bool insert(Timer* timer);



};