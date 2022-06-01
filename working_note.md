# 重要参考

[muduo源码剖析](https://zhuanlan.zhihu.com/p/85101271)

《Linux多线程服务器端编程》

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

1 父线程：ThreadPool::start()创建Thread对象（此时pthread还未被创建），随后调用Thread::startLoop()，Thread::startLoop()之后的调用关系：Thread::startLoop() -> Thread::start()(这一步创建pthread) -> 

子线程：pthread创建后调用::startThread() -> ThreadData::runInThread() -> ThreadData::func_() (也就是Thread::threadFunc)  -> Thread::reactor.loop()



问题：

1 还是不理解ThreadData的作用

2 为什么需要用CountDownLatch控制 "确保func_确实执行后，Thread::start()才返回"

将latch.wait注释掉，也并没有报错（考虑到和原项目的统一性，还是保留Latch）

## ThreadPool类

基本完成了ThreadPool（能够启多个线程）

TODO: 开始写Reactor（需要先写Handler类）



# Day3 20220515

## 原项目Channel类的分析

实际进行监听，获取事件发生消息的仍然是Reactor类。原先的实现耦合了Event和Handler（称为channel）

理论上，伪代码应该是

```
for event in new_events
	handler.handle(event)
```

原项目的伪代码是

```
for channel in channels
	channel.handle()
```

![并发模型](media/model.png)

（注意主从reactor架构下，从reactor可以有一个，也可以有多个）

（突然能够理解为什么原项目命名为channel了，因为channel本质上是main Reactor 交给SubReactor的已创建连接，相当于是和远端的用户建立了信道（channel））

（进一步拓展原来的想法，channel还是主reactor去唤醒从reactor的“信道”，其不止关联了一个事件（比如可能包含读事件, 后续可能会有写事件等））

（因此，原项目实际上有两类Channel，一类是主Reactor与从Reactor之间的Channel，在这里出现的事件是主Reactor要将接收的连接分配给从Reacotr，对从Reactor唤醒；另一类是从Reactor和Web客户端之间的Channel，这里出现的事件就是HTTP读写事件）

（原项目中 EventLoop 和 HttpData的联系方式就是将HttpData对应的文件描述符加入到poller_监听的列表里）

## Epoll类

封装了Linux的epoll相关方法

目前完成了epoll_create和poll()方法



# Day4 20220516

## 架构变更

为什么原项目把Event+Handler打包放在Channel里？**因为将Event的Handle方式通过泛型函数的方式指定，是合理的。外部用户可以直接对每一个单独的event绑定相应的handler。**从而可以直接通过event->handle去调用其处理方法。

如果分开来，handler就需要维护一个mapping，映射事件类型到相应的处理函数，每次调用都需要做映射后调用，并不自然。

重新使用Channel的名称，但是将原先的HttpData改为HttpChannel，明确其作用。

## Channel类

**Channel表示“事件所发生在的通信信道”**

原项目中的HttpData类实际上封装了 “subReactor到用户的TCP/Http通信信道”

原项目中的Server类包含了对新连接处理的回调函数

## Reactor类

### eventfd系统调用

> eventfd是linux 2.6.22后系统提供的一个轻量级的进程间通信的系统调用，eventfd通过一个进程间共享的64位计数器完成进程间通信，这个计数器在linux内核空间维护，用户可以通过调用write方法向内核空间写入一个64位的值，也可以调用read方法读取这个值。

新建

```cpp
#include <sys/eventfd.h>
int eventfd(unsigned int initval, int flags);
//flags:
//EFD_CLOEXEC : fork子进程时不继承，对于多线程的程序设上这个值不会有错的。
//EFD_NONBLOCK: 文件会被设置成O_NONBLOCK，读操作不阻塞。若不设置，一直阻塞直到计数器中的值大于0。
//EFD_SEMAPHORE : 支持 semophore 语义的read，每次读操作，计数器的值自减1。
```

读操作(读计数器中的值)

```cpp
typedef uint64_t eventfd_t;
int eventfd_read(int fd, eventfd_t *value);
```

1. 如果计数器中的值大于0：

- 设置了 `EFD_SEMAPHORE` 标志位，则返回1，且计数器中的值也减去1。
- 没有设置 `EFD_SEMAPHORE` 标志位，则返回计数器中的值，且计数器置0。

1. 如果计数器中的值为0：

- 设置了 `EFD_NONBLOCK` 标志位就直接返回-1。
- 没有设置 `EFD_NONBLOCK` 标志位就会一直阻塞直到计数器中的值大于0。

写操作(向计数器中写入值)

```cpp
int eventfd_write(int fd, eventfd_t value);
```

1. 如果写入值的和小于0xFFFFFFFFFFFFFFFE，则写入成功
2. 如果写入值的和大于0xFFFFFFFFFFFFFFFE

- 设置了 `EFD_NONBLOCK` 标志位就直接返回-1。
- 如果没有设置 `EFD_NONBLOCK` 标志位，则会一直阻塞直到read操作执行



### 主Reactor分派新连接

主Reactor也进行Reactor::loop()，所以要么是写在event的handle函数中，要么是写到pendingFunctors中。

前者依赖于event read handler的重写，新建立ListenChannel类，专用于处理这部分事件。存在一个问题：所注册的事件处理函数并不是虚函数，这里强行使用继承的方式调用，需要强制类型转换，并且很不优雅。还有一种方式是不应用继承的方式，而是建立“组合”的关系。此时可以不变动原先Channel-> handleEvent()的处理。



# Day6 20220518

将原先的HttpData改为HttpHandle

## HttpHandle类



### 无法处理多个连接

只有第一个连接可以正常建立，第二个连接一来就阻塞了？（已经解决，还是没有设置fd状态的问题）

### 已建立连接关闭时处理异常

无限循环在HttpChannel::handleRead 和 HttpChannel::handleConn中。（已解决，在HttpHandleConn的地方删除Fd）

# Day7 20220520 

TODO: 

0 架构上，确认原项目中的pendingFunctors到底是干什么的。

在Server::handleNewConn中，EventLoop::queueInLoop会将对HttpChannel的fd.events设置以及addToPoller这两个动作对应的函数放到**子Reactor**的pendingFunctors里面。

不清楚为什么不直接在主Reactor直接执行上面两个动作。



1 完成基础的HTTP连接？

边缘触发：只会在状态变化时被触发（只触发一次）

> **水平触发(level-triggered，也被称为条件触发)LT:** 只要满足条件，就触发一个事件(只要有数据没有被获取，内核就不断通知你)
> **边缘触发(edge-triggered)ET:** 每当状态变化时，触发一个事件
>
> > 举个读socket的例子，假定经过长时间的沉默后，现在来了100个字节，这时无论边缘触发和条件触发都会产生一个read ready notification通知应用程序可读。应用程序读了50个字节，然后重新调用api等待io事件。这时水平触发的api会因为还有50个字节可读从 而立即返回用户一个read ready notification。而边缘触发的api会因为可读这个状态没有发生变化而陷入长期等待。 因此在使用边缘触发的api时，要注意每次都要读到socket返回EWOULDBLOCK为止，否则这个socket就算废了。而使用条件触发的api 时，如果应用程序不需要写就不要关注socket可写的事件，否则就会无限次的立即返回一个write ready notification。大家常用的select就是属于水平触发这一类，长期关注socket写事件会出现CPU 100%的毛病。

Http Request 示例

```
GET / HTTP/1.1
Host: 10.176.35.12:8000
User-Agent: curl/7.58.0
Accept: */*
```

# Day8 20220521

## 解决Http读写的一些问题

出现了writen后客户端无法接收到即时返回消息的情况

在对文件描述符的写操作加上\r\n后可以解决。

但这并不合理......

实测直接用socket的recv操作，可以不加\r\n就能顺利读出

查到最后发现是HttpRequest没有带ContentLength，导致读出不正常

添加上正确的ContentLength后读出正常

# Day9 20220522

## 总结一下目前的问题和后续的实现

问题

### 1 还是没搞清楚pendingFunctors到底是做什么用的？

总结一下，”希望把某些动作放到IO线程来做，以避免出现线程安全问题“

在我们的HttpHandle中，实际上就是把add_to_poller的操作放到了IO线程来做，避免出现线程安全问题？是这样吗？

并且，原项目包含了对Timer的操作，书本中，Timer操作是铁定要保证线程安全，因此会统一放在所属IO线程上去完成。

而在我们的项目中，考虑到存在以下的数据结构（epollFd_需要维护为红黑树的结构），因此同样需要避免线程安全问题，所以需要使用pendingFunctors的方式来进入IO线程

```cpp
std::unordered_map<int, std::shared_ptr<Channel>> fd2channel;
void Epoll::epoll_add(std::shared_ptr<Channel> channel, int timeout){ //添加所要监听的文件描述符
    int eventFd = channel->fd_;
    //TODO: Add timer 
    struct epoll_event ep_event; //The events member is a bit mask composed by ORing together zero or more of the following available event types
    ep_event.data.fd = eventFd;  
    ep_event.events = channel->eventFlag_;
    fd2channel[eventFd] = channel;
    if(epoll_ctl(epollFd_, EPOLL_CTL_ADD, eventFd, &ep_event) < 0){
        std::cout << "Error occur in epoll_add" << std::endl;
    }
}
```



### 2 在线程和线程库的地方，没搞清楚那些锁的作用

Thread中大量使用了Mutex。

主要是为了确保线程创建过程中的正确性（保证子线程运行到相应位置后，父线程才继续运行后面的代码）

（结合书本考虑一下）

## TODO

1 实现连接关闭的正确处理（目前似乎在epoll_delete的位置还是有问题）

2 refractor一下

# Day10 20220524

## 尝试实现优雅的连接关闭

### 关于close 和 epoll_ctl(delete)

>  ![img](https://pic1.zhimg.com/80/7d4036832aaa88d7b5c3af0a2e89ebb2_720w.jpg?source=1940ef5c)



>  感觉题主其实是在问：epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&amp;ev); 和 close(sockfd); 这两句话有必要同时存在吗？

>  要是你能确定你的close不是只减少了fd的一个引用计数，而是实打实地把引用计数从1减为0（只有这时close才会真正地去close一个fd），那才能说epoll会自动把这个socket的fd从监听队列中删除。

> 所以EPOLL_CTL_DEL的作用就是确保把这个socket的fd从监听队列中删除，不管你的close到底只是减少了一个引用计数还是真正地去close了一下。

> 否则注册EPOLL_CTL_DEL事件岂不就真的毫无用处了吗？

> 作者：文武双缺
> 链接：https://www.zhihu.com/question/19804426/answer/96510492
> 来源：知乎
> 著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

### 修正方式

将原先epoll_ctl和close共用的方式改为了仅使用close，能够避免error in close

# Day11 20220525

还有什么可以做的吗？
