/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-04-09 00:57:42
 * @LastEditors: closing
 * @LastEditTime: 2023-05-22 10:25:40
 * @FilePath: /sylar/src/utils.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <execinfo.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <google/protobuf/unknown_field_set.h>
#include "utils.h"
#include "logger.h"
#include "fiber.h"
namespace server_cc{
    
//获取线程id
pid_t GetThreadId(){
    return syscall(SYS_gettid);
}
uint32_t GetFiberId(){

    // SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT())<<"not get fiberId "<<std::endl;
    return server_cc::Fiber::GetFiberId();
}

uint64_t GetCurrentMS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ul  + tv.tv_usec / 1000;
}

uint64_t GetCurrentUS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000ul  + tv.tv_usec;
}

void BackTrace(std::vector<std::string>&bt, int size,int skip){
    void** buffer=(void**)malloc(sizeof(void*)*size);
    size_t size_=backtrace(buffer,size);
    char** string=backtrace_symbols(buffer,size_);
    for(size_t i=skip;i<size_;i++){
        bt.push_back(string[i]);
    }
    free(string);
    free(buffer);
}
/**
 * 
*/
std::string BackTraceToString(int size, int skip, const std::string&prefix){
    std::vector<std::string> bt;
    BackTrace(bt,size,skip);
    std::stringstream ss;
    for(int i=0;i<bt.size();i++){
        ss<<prefix<<bt[i]<<std::endl;
    }
    return ss.str();
}

}