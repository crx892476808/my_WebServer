#include "Timer.h"

int64_t Timer::s_numCreated_ = 1;

void Timer::restart(Timestamp now){
    if(repeat_){
        expiration_ = addTime(now, interval_);
    }
    else{
        expiration_ = Timestamp(); // an invalid value
    }
}