/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-09 17:00:53
 * @LastEditors: closing-f fql2018@bupt.edu.cn
 * @LastEditTime: 2023-05-10 09:13:47
 * @FilePath: /src/test_fiber.cpp
 * @Description: 
 */
#include "src/logger.h"
#include "src/thread.h"
#include "src/config.h"
#include "src/mutex.h"
#include "src/fiber.h"
#include "src/utils.h"
#include <yaml-cpp/yaml.h>

server_cc::Logger::ptr g_logger = SEVER_CC_LOG_ROOT();

void run_in_fiber() {
    SEVER_CC_LOG_INFO(g_logger) << "run_in_fiber begin";
    server_cc::Fiber::YieldToHold();
    SEVER_CC_LOG_INFO(g_logger) << "run_in_fiber end";
    server_cc::Fiber::YieldToHold();
}

void test_fiber() {
    SEVER_CC_LOG_INFO(g_logger) << "main begin -1";
    {
        server_cc::Fiber::GetThis();
        SEVER_CC_LOG_INFO(g_logger) << "main begin";
        server_cc::Fiber::ptr fiber(new server_cc::Fiber(run_in_fiber));
        fiber->swapIn();
        SEVER_CC_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        SEVER_CC_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    SEVER_CC_LOG_INFO(g_logger) << "main after end2";
}

int main(int argc, char** argv) {
    server_cc::Thread::SetName("main");

    std::vector<server_cc::Thread::ptr> thrs;
    for(int i = 0; i < 1; ++i) {
        thrs.push_back(server_cc::Thread::ptr(
                    new server_cc::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    return 0;
}
