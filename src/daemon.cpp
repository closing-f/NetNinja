/*
 * @Author: closing
 * @Date: 2023-06-20 22:47:04
 * @LastEditors: closing
 * @LastEditTime: 2023-07-10 10:44:25
 * @Description: 请填写简介
 */
#include "daemon.h"
#include "logger.h"
#include "config.h"
#include<time.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>

namespace server_cc{

static server_cc::Logger::ptr g_logger = SERVER_CC_LOG_NAME("system");
static server_cc::ConfigVar<uint32_t>::ptr g_daemon_restart_interval = 
    server_cc::Config::Lookup("daemon.restart_interval",(uint32_t)5,"daemon restart interval");

std::string ProcessInfo::toString() const {
    std::stringstream ss;
    ss << "[ProcessInfo parent_pid=" << parent_pid
       << " main_pid=" << main_pid
       << " parent_start_time=" << server_cc::Time2Str(parent_start_time)
       << " main_start_time=" << server_cc::Time2Str(main_start_time)
       << " restart_count=" << restart_count << "]";
    return ss.str();
}

/**
 * @description: 最终运行的程序
 * @return {*}
 */
static int real_start(int argc, char** argv,
                     std::function<int(int argc, char** argv)> main_cb) {
    ProcessInfoMgr::GetInstance().main_pid = getpid();
    ProcessInfoMgr::GetInstance().main_start_time = time(0);
    return main_cb(argc, argv);
}


/**
 * @description: 以守护进程的方式运行程序
 * @param {int} argc
 * @param {char**} argv
 * @param {function<int(int argc,char** argv)>} main_cb
 * @return {*}
 */
static int real_daemon(int argc, char** argv,
                     std::function<int(int argc, char** argv)> main_cb) {
    // If nochdir is zero, daemon() changes the process's current working directory to the root directory ("/"); otherwise, the current working directory is left unchanged.
    // If noclose is zero, daemon() redirects standard input, standard output and standard error to /dev/null; otherwise, no changes are made to these file descriptors.
    daemon(1, 0);
    ProcessInfoMgr::GetInstance().parent_pid = getpid();
    ProcessInfoMgr::GetInstance().parent_start_time = time(0);
    while(true) {
        pid_t pid = fork();
        if(pid == 0) {
            //子进程返回
            ProcessInfoMgr::GetInstance().main_pid = getpid();
            ProcessInfoMgr::GetInstance().main_start_time  = time(0);
            SERVER_CC_LOG_INFO(g_logger) << "process start pid=" << getpid();
            return real_start(argc, argv, main_cb);
        } else if(pid < 0) {
            SERVER_CC_LOG_ERROR(g_logger) << "fork fail return=" << pid
                << " errno=" << errno << " errstr=" << strerror(errno);
            return -1;
        } else {
            //父进程返回
            int status = 0;
            SERVER_CC_LOG_INFO(g_logger) << "wait pid=" << pid;
            waitpid(pid, &status, 0);
            if(status) {
                if(status == 9) {
                    SERVER_CC_LOG_INFO(g_logger) << "killed";
                    break;
                } else {
                    SERVER_CC_LOG_ERROR(g_logger) << "child crash pid=" << pid
                        << " status=" << status;
                }
            } else {
                SERVER_CC_LOG_INFO(g_logger) << "child finished pid=" << pid;
                break;
            }
            ProcessInfoMgr::GetInstance().restart_count += 1;
            sleep(g_daemon_restart_interval->getValue());
        }
    }
    return 0;
}

int start_daemon(int argc, char** argv, std::function<int(int argc,char** argv)> main_cb,bool is_daemon) {
    if(!is_daemon) {
        ProcessInfoMgr::GetInstance().parent_pid = getpid();
        ProcessInfoMgr::GetInstance().parent_start_time = time(0);
        return real_start(argc, argv, main_cb);
    }
    return real_daemon(argc, argv, main_cb);
}
}//namespace server_cc