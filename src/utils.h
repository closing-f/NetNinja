/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-04-09 00:57:42
 * @LastEditors: closing
 * @LastEditTime: 2023-05-22 15:50:02
 * @FilePath: /sylar/src/utils.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
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
#include<vector>
#include<string>
#include <iostream>
#include <iomanip>
#include <execinfo.h>	/* for backtrace() */
#include <assert.h>


//? 下面if 中 x两边必须加括号，为啥
#define SERVER_CC_ASSERT(x) \
    if(!(x)){\
        SERVER_CC_LOG_ERROR(SERVER_CC_LOG_ROOT()) << "ASSERTION: "<< #x \
            << "\nbacktrace:\n" \
            << server_cc::BackTraceToString(100, 2, "    "); \
        assert(x); \
    }
//? #x 和 x的差别
#define SERVER_CC_ASSERT2(x,w) \
    if(!(x)){\
        SERVER_CC_LOG_ERROR(SERVER_CC_LOG_ROOT()) << "ASSERTION: "<< #x \
            << "\n"<< w \
            << "\nbacktrace:\n" \
            << server_cc::BackTraceToString(100, 2, "    "); \
        assert(x); \
    }


namespace server_cc{

pid_t GetThreadId();

uint32_t GetFiberId();
/**
 * @brief 获取当前时间的毫秒
 */
uint64_t GetCurrentMS();

/**
 * @brief 获取当前时间的微秒
 */
uint64_t GetCurrentUS();
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
/**
 * @
*/
/**
 * @description: 
 * @param {int} size
 * @param {int} skip
 * @return {*}
 */
void BackTrace(std::vector<std::string>&bt,int size=64,int skip=1);

/**
 * @description: 
 * @param {int} size
 * @param {int} skip
 * @return {*}
 */
std::string BackTraceToString(int size=64,int skip=2,const std::string &prefix="");


}
#endif // !


