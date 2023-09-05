/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-04-09 00:57:42
 * @LastEditors: closing
 * @LastEditTime: 2023-07-10 09:47:45
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

std::string Time2Str(time_t ts, const std::string& format) {
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

time_t Str2Time(const char* str, const char* format) {
    struct tm t;
    memset(&t, 0, sizeof(t));
    if(!strptime(str, format, &t)) {
        return 0;
    }
    return mktime(&t);
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
static int __lstat(const char* file, struct stat* st = nullptr) {
    struct stat lst;
    int ret = lstat(file, &lst);
    if(st) {
        *st = lst;
    }
    return ret;
}
void FSUtil::ListAllFile(std::vector<std::string>& files, const std::string& path, const std::string& subfix){
    
    // access()函数用于判断文件是否存在，存在返回0，不存在返回-1.
    //If pathname is a symbolic link, it is dereferenced.
    if(access(path.c_str(),0)!=0){
        return;
    }
    DIR* dir=opendir(path.c_str());
    if(dir==nullptr){
        return;
    }   
    struct dirent* dp=nullptr;
    while((dp=readdir(dir))!=nullptr){
        if(dp->d_type==DT_DIR){
            if(!strcmp(dp->d_name,".")||!strcmp(dp->d_name,"..")){
                continue;
            }
            ListAllFile(files,path+"/"+dp->d_name,subfix);
        }else if(dp->d_type==DT_REG){
            std::string filename(dp->d_name);
            if(subfix.empty()){
                files.push_back(path+"/"+filename);
            }else{
                if(filename.size()<subfix.size()){
                    continue;
                }
                if(filename.substr(filename.length()-subfix.size())==subfix){
                    files.push_back(path+"/"+filename);
                }
            }
        }
    }
    closedir(dir);
}


bool FSUtil::IsRunningPidfile(const std::string& pidfile) {
    if(__lstat(pidfile.c_str()) != 0) {
        return false;
    }
    std::ifstream ifs(pidfile);
    std::string line;
    if(!ifs || !std::getline(ifs, line)) {
        return false;
    }
    if(line.empty()) {
        return false;
    }
    pid_t pid = atoi(line.c_str());
    if(pid <= 1) {
        return false;
    }
    if(kill(pid, 0) != 0) {
        return false;
    }
    return true;
}

static int __mkdir(const char* dirname) {
    if(access(dirname, F_OK) == 0) {
        return 0;
    }
    return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

bool FSUtil::Mkdir(const std::string& dirname) {
    if(__lstat(dirname.c_str()) == 0) {
        return true;
    }
    char* path = strdup(dirname.c_str());
    char* ptr = strchr(path + 1, '/');
    do {
        for(; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
            *ptr = '\0';
            if(__mkdir(path) != 0) {
                break;
            }
        }
        if(ptr != nullptr) {
            break;
        } else if(__mkdir(path) != 0) {
            break;
        }
        free(path);
        return true;
    } while(0);
    free(path);
    return false;
}
} // namespace server_cc