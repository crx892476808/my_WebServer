# Day0 20220501-20220509

先将整个项目大致读了一下，加上gdb调试，有了一些自己的理解。

（这WebServer是没有数据库的，直接将网页元素打包后写到fd里）

# Day1 20220510 15:12

自底向上实现原先的代码

### MutexLock类

封装了pthread_mutex_t类型（互斥锁）

- pthread_mutex_t 类型

头文件

```cpp
#include<pthread.h>
```

1 互斥锁的创建和销毁

> 有两种方法创建互斥锁，**静态方式**和**动态方式**。POSIX定义了一个宏**PTHREAD_MUTEX_INITIALIZER**来静态初始化互斥锁，方法如下：
>
> ```cpp
> pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
> ```
>
> 在LinuxThreads的实现中，pthread_mutex_t是一个结构，而PTHREAD_MUTEX_INITIALIZER则是一个结构体常量
>
> 动态方式是采用**pthread_mutex_init()**函数来初始化互斥锁，API定义如下：
>
> ```cpp
> int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
> ```
>
> 其中*mutexattr*用于指定互斥锁属性（见下），如果为**NULL**则使用缺省属性。互斥锁属性：
>
> \* PTHREAD_MUTEX_TIMED_NP，这是缺省值，也就是**普通锁**。*当一个线程加锁以后，其余请求锁的线程将形成一个等待队列，并在解锁后按优先级获得锁*。这种锁策略保证了资源分配的公平性。
> \* PTHREAD_MUTEX_RECURSIVE_NP，**嵌套锁**，*允许同一个线程对同一个锁成功获得多次，并通过多次unlock解锁*。如果是不同线程请求，则在加锁线程解锁时重新竞争。
> \* PTHREAD_MUTEX_ERRORCHECK_NP，**检错锁**，如果*同一个线程请求同一个锁，则返回EDEADLK*，否则与PTHREAD_MUTEX_TIMED_NP类型动作相同。这样保证当不允许多次加锁时不出现最简单情况下的死锁。
> \* PTHREAD_MUTEX_ADAPTIVE_NP，**适应锁**，动作最简单的锁类型，仅**等待解锁后重新竞争**。
>
> **pthread_mutex_destroy()**用于注销一个互斥锁，API定义如下：
>
> ```cpp
> int pthread_mutex_destroy(pthread_mutex_t *mutex)
> ```
>
> 销毁一个互斥锁即意味着释放它所占用的资源，且要求锁当前处于开放状态。由于在Linux中，互斥锁并不占用任何资源，因此LinuxThreads中的 pthread_mutex_destroy()除了检查锁状态以外（锁定状态则返回EBUSY）没有其他动作。

2 加锁和解锁

> 锁操作主要包括加锁pthread_mutex_lock()、解锁pthread_mutex_unlock()和测试加锁 pthread_mutex_trylock()三个，不论哪种类型的锁，都不可能被两个不同的线程同时得到，而必须等待解锁。**对于普通锁和适应锁类型，解锁者可以是同进程内任何线程；而检错锁则必须由加锁者解锁才有效，否则返回EPERM(权限不足)**；对于嵌套锁，文档和实现要求必须由加锁者解锁，但实验结果表明并没有这种限制，这个不同目前还没有得到解释。在同一进程中的线程，如果加锁后没有解锁，则任何其他线程都无法再获得锁。
>
> ```cpp
> 　　int pthread_mutex_lock(pthread_mutex_t *mutex)
> 　　int pthread_mutex_unlock(pthread_mutex_t *mutex)
> 　　int pthread_mutex_trylock(pthread_mutex_t *mutex)
> ```
>
> 　　**pthread_mutex_trylock()语义与pthread_mutex_lock()类似，不同的是在锁已经被占据时返回EBUSY而不是挂起等待。**

### Condition类

封装了Linux中的pthread_cond_init类

> 1 初始化条件变量pthread_cond_init
>
> ```cpp
> #include <pthread.h>
> 
> int pthread_cond_init(pthread_cond_t *cv, const pthread_condattr_t *cattr);
> ```
>
> 返回值：函数成功返回0，任何其他的返回值都表示错误
>
> 初始化一个条件变量。当参数cattr为空指针时，函数创建的是一个缺省的条件变量。否则条件变量的属性将由cattr中的属性值来决定。
>
> 不能由多个线程同时初始化一个条件变量。当需要重新初始化或释放一个条件变量时，应用程序必须保证这个条件变量未被使用。
>
> 2 阻塞在条件变量上pthread_cond_wait
>
> ```cpp
> #include <pthread.h>
> int pthread_cond_wait(pthread_cond_t *cv, pthread_mutex_t *mutex)
> ```
>
> 返回值：函数成功返回0；任何其他返回值都表示错误
>
> 函数将解锁mutex参数指向的互斥锁，并使当前线程阻塞在cv参数指向的条件变量上。被阻塞的线程可以被pthread_cond_signal函数，pthread_cond_broadcast函数唤醒，也可能在被信号中断后被唤醒。
>
> pthread_cond_wait函数的返回并不意味着条件的值一定发生了变化，必须重新检查条件的值。**pthread_cond_wait调用时，将调用线程放在条件等待队列，对互斥量解锁。pthread_cond_wait函数返回时，相应的互斥锁将被当前线程锁定(因为后续要判断条件，需要对条件加锁)，即使是函数出错返回**。
>
> 一般一个条件表达式都是在一个互斥锁的保护下被检查。当条件表达式未被满足时，线程将仍然阻塞在这个条件变量上。**当另一个线程改变了条件的值并向条件变量发出信号时，等待在这个条件变量上的一个线程或所有线程被唤醒，接着都试图再次占有相应的互斥锁**。**阻塞在条件变量上的线程被唤醒以后，直到pthread_cond_wait()函数返回之前条件的值都有可能发生变化。**所以函数返回以后，在锁定相应的互斥锁之前，必须重新测试条件值。**最好的测试方法是循环调用pthread_cond_wait函数，并把满足条件的表达式置为循环的终止条件。**如：
>
> ```cpp
> pthread_mutex_lock();
> 
> while (condition_is_false)
> 	pthread_cond_wait();
> 
> pthread_mutex_unlock();
> ```
>
> 注意，阻塞在同一个条件变量上的不同线程被释放的次序是不一定的。
>
> 3 解除在条件变量上的阻塞pthread_cond_signal
>
> ```cpp
> #include<pthread.h>
> int pthread_cond_signal(pthread_cond_t *cv);
> ```
>
> 函数被用来释放被阻塞在指定条件变量上的一个线程。返回值：函数成功返回0；任何其他返回值都表示错误。必须在互斥锁的保护下使用相应的条件变量。否则对条件变量的解锁有可能发生在锁定条件变量之前，从而造成死锁。

### Thread类

```cpp
typedef std::function<void()> ThreadFunc;
```

> std::function 既可以包装函数，也可以包装函数指针和lambda表达式

```cpp
namespace CurrentThread {
	__thread int t_cachedTid = 0;
	__thread char t_tidString[32];
	__thread int t_tidStringLength = 6;
	__thread const char* t_threadName = "default";
}
```

> Thread Local Storage（tls，线程局部存储）是一种机制，**通过这一机制分配的变量，每个当前线程有一个该变量的实例**。该机制需要链接器(ld)，动态链接器(ld.so)和系统库(libc.so)的全力支持，因此它不是到处可用的。**在用户层，用一个新的存储类型关键字：`__thread`表示这一扩展，可以用来修饰那些带有全局性且值可能变，但是又不值得用全局变量保护的变量。**`__thread`限定符可以单独使用，也可以带有extern或static限定符，但不能带有其他存储类型的限定符。`__thread`可用于全局的静态文件作用域，静态函数作用域或一个类中的静态数据成员。不能用于块作用域，自动或非静态数据成员。
>
> 原理：
>
> 当应用address-of操作符于一个线程局部变量时，它被在运行时求值，返回该变量当前线程实例的地址。这样得到的地址可以被其他任何线程使用。当一个线程终止时，任何该线程中的线程局部存储变量都不再有效。在c++中，如果一个线程局部存储变量有一个初始化器，它必须是常量表达式。
>
> 注意事项：
>
> 1 只能修饰POD类型（类似整型指针的标量，不带自定义的构造，拷贝，赋值，析构的类型，二进制内容可以任意赋值memset，memcpy，且内容可以复原）。不能修饰class类型，因为无法自动调用构造函数和析构函数。

```cpp
prctl(PR_SET_NAME, “process_name”);
```

> 第一个参数是操作类型，指定PR_SET_NAME，即设置进程名
>
> 第二个参数是进程名字符串，长度至多16字节

```cpp
pthread_create(&pthreadId_, NULL, &startThread, data)
```

> ```cpp
> int pthread_create(pthread_t *thread,
>                 const pthread_attr_t *attr,
>                 void *(*start_routine) (void *),
>                 void *arg);
> ```
>
> 各个参数的含义是：
> \1) pthread_t *thread：传递一个 pthread_t 类型的指针变量，也可以直接传递某个 pthread_t 类型变量的地址。pthread_t 是一种用于表示线程的数据类型，每一个 pthread_t 类型的变量都可以表示一个线程。
>
> \2) const pthread_attr_t *attr：用于手动设置新建线程的属性，例如线程的调用策略、线程所能使用的栈内存的大小等。大部分场景中，我们都不需要手动修改线程的属性，将 attr 参数赋值为 NULL，pthread_create() 函数会采用系统默认的属性值创建线程。
>
> pthread_attr_t 类型以结构体的形式定义在`<pthread.h>`头文件中，此类型的变量专门表示线程的属性。关于线程属性，您可以阅读《[线程属性有哪些，如何自定义线程属性？](http://c.biancheng.net/thread/vip_8620.html)》一文做详细地了解。
>
> \3) void *(*start_routine) (void *)：以函数指针的方式指明新建线程需要执行的函数，该函数的参数最多有 1 个（可以省略不写），形参和返回值的类型都必须为 void* 类型。void* 类型又称空指针类型，表明指针所指数据的类型是未知的。使用此类型指针时，我们通常需要先对其进行强制类型转换，然后才能正常访问指针指向的数据。
>
> > 如果该函数有返回值，则线程执行完函数后，函数的返回值可以由 pthread_join() 函数接收。有关 phtread_join() 函数的用法，我们会在《[获取线程函数返回值](http://c.biancheng.net/thread/vip_8627.html)》一节给大家做详细讲解。
>
> \4) void *arg：指定传递给 start_routine 函数的实参，当不需要传递任何数据时，将 arg 赋值为 NULL 即可。
>
> 如果成功创建线程，pthread_create() 函数返回数字 0，反之返回非零值。各个非零值都对应着不同的宏，指明创建失败的原因，常见的宏有以下几种：
>
> - EAGAIN：系统资源不足，无法提供创建线程所需的资源。
> - EINVAL：传递给 pthread_create() 函数的 attr 参数无效。
> - EPERM：传递给 pthread_create() 函数的 attr 参数中，某些属性的设置为非法操作，程序没有相关的设置权限。
>
> **从程序的执行过程不难看出， pthread_create() 函数成功创建的线程会自动执行指定的函数，不需要手动开启。**

TODO: 结合原项目的EventLoopThread，把Thread完成

# Day2 20220511

## Thread类（Cont.）

笔记：

1 Thread::startLoop()之后的调用关系：Thread::startLoop() -> Thread::start()(这一步创建新线程) -> ::startThread() -> ThreadData::runInThread() -> func_() (也就是reactor->loop()) 

2 在最初始的位置，会由ThreadPool::start()调用Thread::startLoop()

问题：

1 还是不理解ThreadData的作用

2 为什么需要用CountDownLatch控制 "确保func_确实执行后，Thread::start()才返回"



## ThreadPool类

