/*
 * @Author: closing
 * @Date: 2023-07-10 10:18:48
 * @LastEditors: closing
 * @LastEditTime: 2023-07-10 10:21:24
 * @Description: 请填写简介
 */
#include "src/daemon.h"
#include "src/iomanager.h"
#include "src/logger.h"

static server_cc::Logger::ptr g_logger = SERVER_CC_LOG_ROOT();

server_cc::Timer::ptr timer;
int server_main(int argc, char** argv) {
    SERVER_CC_LOG_INFO(g_logger) << server_cc::ProcessInfoMgr::GetInstance().toString();
    server_cc::IOManager iom(1);
    timer = iom.addTimer(1000, [](){
            SERVER_CC_LOG_INFO(g_logger) << "onTimer";
            static int count = 0;
            std::cout << "count=" << count << std::endl;
            if(++count > 10) {
                exit(1);
            }
    }, true);
    return 0;
}

int main(int argc, char** argv) {
    return server_cc::start_daemon(argc, argv, server_main, argc != 1);
}
