#ifndef UTILS_H
#define UTILS_H
#include<sys/types.h> 
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
namespace server_cc{
    pid_t GetThreadId();
    uint32_t GetFiberId();

template<class T,typename X=void,int N=0>
class Singleton{
    public:
        static T& GetInstance(){
            //首次调用时，进行初始化，后面调用时，直接返回，
            //它始终驻留在全局数据区，直到程序运行结束。但其作用域为局部作用域，当定义它的函数或语句块结束时，其作用域随之结束。
            static T v;
            return v;
        }
};

}
#endif // !


