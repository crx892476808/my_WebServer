/*** 
 * @Author: Armin Jager
 * @Date: 2022-05-17 08:26:41
 * @LastEditTime: 2022-05-18 19:25:32
 * @LastEditors: Armin Jager
 * @Description: Date +8h
 */

#include"Util.h"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>

static const int MAX_BUFF = 4096; //全局变量默认是在其它文件可见的，加上static关键字确保变量只在本文件可见

ssize_t writen(int fd, void *buff, size_t n){//write nLeft Bytes of buff to fd, 返回实际写入的字节数
    size_t nLeft = n; //剩余还需写的字节数
    ssize_t nWritten = 0;
    ssize_t nWriteSum = 0;
    char *ptr = (char*)buff;
    while(nLeft > 0){
        nWritten = write(fd, ptr, nLeft);
        if(nWritten <= 0){ //write nLeft Bytes of buff to fd
            if(errno == EINTR){ //errno 用来保存最后的错误代码; EINTR:Interrupted system call，由于信号中断，没写成功任何数据
                nWritten = 0;
                continue;
            }
            else if(errno == EAGAIN){ //Resource temporarily unavailable, 从字面上来看，是提示再试一次(有可能是发送缓冲区已满)
                return nWriteSum;
            }
            else{
                return -1;
            }
        }
        nLeft -= nWritten;
        nWriteSum += nWritten;
        ptr += nWritten;

    }
    return nWritten;
}

ssize_t writen(int fd, std::string &str){
    size_t nLeft = str.size(); //剩余还需写的字节数
    ssize_t nWritten = 0;
    ssize_t nWriteSum = 0;
    const char *buff = str.c_str();
    while(nLeft > 0){
        
        nWritten = write(fd, (const void*)buff, nLeft);
        if(nWritten <= 0){
            if(errno == EINTR){ //errno 用来保存最后的错误代码; EINTR:Interrupted system call，由于信号中断，没写成功任何数据
                nWritten = 0;
                continue;
            }
            else if(errno == EAGAIN){ //Resource temporarily unavailable, 从字面上来看，是提示再试一次(有可能是发送缓冲区已满)
                break;//return nWriteSum;
            }
            else{
                return -1;
            }
        }
        nWriteSum += nWritten;
        nLeft -= nWritten;
        buff += nWritten;
    }
    if(nWriteSum == static_cast<int>(str.size())){
        str.clear();
    }
    else{
        str = str.substr(nWriteSum);
    }
    return nWriteSum;
}

ssize_t readn(int fd, void *buff, size_t n){
    size_t nLeft = n;
    ssize_t nRead = 0;
    ssize_t nReadSum = 0;
    char *ptr = (char*)buff;
    while(nLeft > 0){
        nRead = read(fd, buff, nLeft);
        if(nRead <= 0){
            if(errno == EINTR){ //errno 用来保存最后的错误代码; EINTR:Interrupted system call，由于信号中断，没写成功任何数据
                nRead = 0;
                continue;
            }
            else if(errno == EAGAIN){ //Resource temporarily unavailable, 从字面上来看，是提示再试一次(有可能是发送缓冲区已满)
                return nRead;
            }
            else{
                return -1;
            }
        }
        nLeft -= nRead;
        nReadSum += nRead;
        ptr += nRead;
    } 
    return nRead;
}

ssize_t readn(int fd, std::string &str, bool &readZeroByte){
    ssize_t nRead = 0;
    size_t readSum = 0;
    while(true){
        char buff[MAX_BUFF];
        std::cout << "before readn::read" << std::endl;
        nRead = read(fd, buff, MAX_BUFF);
        std::cout << "after readn::read" << std::endl;
        std::cout << "nRead = " << nRead << std::endl;
        if(nRead < 0){//meets error or finish Reading
            if(errno == EINTR){
                continue;
            }    
            else if(errno == EAGAIN){
                return readSum;
            }
            else{
                return -1;
            }
        }
        else if(nRead == 0){ //0 is for EOF(这里应该是连接关闭的指代)
            readZeroByte = true;
            break;
        }
        readSum += nRead;
        str += std::string(buff, buff + nRead);
        std::cout << "str == " << str << std::endl;
    }
    return readSum;
}

int setSocketNonBlocking(int fd){
  int flag = fcntl(fd, F_GETFL, 0); //Do the file control operation described by CMD on FD.
  if (flag == -1) return -1; //F_GETFL: /* Get file status flags.  */

  flag |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flag) == -1) return -1;
  return 0;
}