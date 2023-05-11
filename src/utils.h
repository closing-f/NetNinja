/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-04-09 00:57:42
 * @LastEditors: closing-f fql2018@bupt.edu.cn
 * @LastEditTime: 2023-05-09 22:38:47
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
#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define SYLAR_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define SYLAR_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define SYLAR_LIKELY(x)      (x)
#   define SYLAR_UNLIKELY(x)      (x)
#endif

//? 下面if 中 x两边必须加括号，为啥
#define SEVER_ASSERT(x) \
    if(!(x)){\
        SEVER_CC_LOG_ERROR(SEVER_CC_LOG_ROOT()) << "ASSERTION: "<< #x \
            << "\nbacktrace:\n" \
            << server_cc::BackTraceToString(100, 2, "    "); \
        assert(x); \
    }
//? #x 和 x的差别
#define SEVER_ASSERT2(x,w) \
    if(!(x)){\
        SEVER_CC_LOG_ERROR(SEVER_CC_LOG_ROOT()) << "ASSERTION: "<< #x \
            << "\n"<< w \
            << "\nbacktrace:\n" \
            << server_cc::BackTraceToString(100, 2, "    "); \
        assert(x); \
    }


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


