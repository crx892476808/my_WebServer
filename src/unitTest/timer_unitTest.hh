#include<iostream>
#include<string>

#include"../Reactor.h"
#include"../mylib/TimerQueue.h"
#include"../mylib/Timestamp.h"
using namespace std;

void timerTestFunc(){
    cout << "timerTestFunc() called at " << Timestamp::now().toFormattedString(false) << endl;
}

void TestTimer(){
    Reactor r1;// r2;
    TimerQueue q1(&r1);
    cout << "addTimer() at " << Timestamp::now().toFormattedString(false) << endl;
    q1.addTimer(timerTestFunc, addTime(Timestamp::now(), 2), 0.0);
    r1.loop();

    
    cout << "TimerUnitTest" << endl;
}