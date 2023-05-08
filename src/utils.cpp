#include "utils.h"
namespace server_cc{
    
    //获取线程id
    pid_t GetThreadId(){
        return syscall(SYS_gettid);
    }
    uint32_t GetFiberId(){
        return 0;
    }
}