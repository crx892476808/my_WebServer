// @Author Armin Jager
// @Email xxbbb@vip.qq.com
#include <string>
#include <iostream>
#include "Server.h"
#include "ThreadPool.h"
int main(int argc, char *argv[])
{
    std::string logPath = "./WebServer.log";
    Reactor *mainReactor = new Reactor();
    ThreadPool threadPool(mainReactor, 3);
    threadPool.start();
}