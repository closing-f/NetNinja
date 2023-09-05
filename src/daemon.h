/*
 * @Author: closing
 * @Date: 2023-06-20 22:35:57
 * @LastEditors: closing
 * @LastEditTime: 2023-07-10 09:22:02
 * @Description: 用守护进程的方式运行程序
 */
#ifndef _DAEMON_H__
#define _DAEMON_H__

#include <unistd.h>
#include<functional>
#include "utils.h"
namespace server_cc{
struct ProcessInfo{
    pid_t parent_pid = 0; //父进程id
    pid_t main_pid = 0; //主进程id
    uint64_t parent_start_time = 0; //父进程启动时间
    uint64_t main_start_time = 0; //主进程启动时间
    uint32_t restart_count = 0; //重启次数
    std::string toString()const;
};

typedef server_cc::Singleton<ProcessInfo> ProcessInfoMgr;


/**
 * @description: 可以以守护进程的方式运行程序
 * @param {int} argc
 * @param {char**} argv
 * @param {function<int(int argc,char** argv)>} main_cb
 * @param {bool} is_daemon
 * @return {*}
 */
int start_daemon(int argc, char** argv
                 , std::function<int(int argc, char** argv)> main_cb
                 , bool is_daemon);

}//namespace sylar


#endif // _DAEMON_H__