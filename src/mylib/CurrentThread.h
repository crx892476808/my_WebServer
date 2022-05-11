/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-10 12:42:12
 * @LastEditTime: 2022-05-11 18:42:51
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */
#pragma once
#include<stdint.h>

namespace CurrentThread{
//internal
extern __thread int t_cachedTid;//extern关键字，表示变量声明
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;
//如过是第一次调用,调用cacheTid(),再调用detail::gettid(),再调用系统的SYS_gettid,获得线程tid赋值给t_cacheTid,也就是获得当前线程id并缓存返回t_cacheTid的值
void cacheTid();
inline int tid(){
    /*这个指令是gcc引入的，作用是允许程序员将最有可能执行的分支告诉编译器。这个指令的写法为：__builtin_expect(EXP, N)。意思是：EXP==N的概率很大。*/
    if(__builtin_expect(t_cachedTid == 0, 0)){
       cacheTid();
    }
    return t_cachedTid;
}

inline const char* tidString(){ //for logging
    return t_tidString;
}

inline int tidStringLength(){ //for logging
    return t_tidStringLength;
}

inline const char* name(){
    return t_threadName;
}
}
