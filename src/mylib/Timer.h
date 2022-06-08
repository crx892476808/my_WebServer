#pragma once
#include "Timestamp.h"
#include <functional>
using std::function;

class Timer{
public:
    typedef function<void()> TimerCallback;
    Timer(TimerCallback cb, Timestamp when, double interval): callback_(cb), expiration_(when), 
        interval_(interval), repeat_(interval> 0.0), sequence_(s_numCreated_++){}
    void run() const { callback_();}
    void restart(Timestamp now);
    Timestamp  expiration() {return expiration_;}
    bool repeat() const {return repeat_;}
    int64_t sequence() {return sequence_;}
private:
    const TimerCallback callback_;
    Timestamp expiration_;//下次过期的时间
    const double interval_;//两次过期的间隔
    const bool repeat_;
    const int64_t sequence_; //唯一序列号?

    static int64_t s_numCreated_;
};