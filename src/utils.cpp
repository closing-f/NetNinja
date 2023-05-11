/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-04-09 00:57:42
 * @LastEditors: closing-f fql2018@bupt.edu.cn
 * @LastEditTime: 2023-05-09 22:38:42
 * @FilePath: /sylar/src/utils.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "utils.h"
#include "logger.h"
#include "fiber.h"
namespace server_cc{
    
//获取线程id
pid_t GetThreadId(){
    return syscall(SYS_gettid);
}
uint32_t GetFiberId(){

    // SEVER_CC_LOG_INFO(SEVER_CC_LOG_ROOT())<<"not get fiberId "<<std::endl;
    return server_cc::Fiber::GetFiberId();
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