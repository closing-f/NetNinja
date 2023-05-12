/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-11 20:00:08
 * @LastEditors: closing-f fql2018@bupt.edu.cn
 * @LastEditTime: 2023-05-12 20:51:01
 * @FilePath: /server_cc/test_scheduler.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "src/logger.h"
#include "src/thread.h"
#include "src/config.h"
#include "src/mutex.h"
#include "src/fiber.h"
#include "src/utils.h"
#include "src/scheduler.h"
#include <yaml-cpp/yaml.h>

static server_cc::Logger::ptr g_logger = SEVER_CC_LOG_ROOT();

void test_fiber() {
    static int s_count = 1;
    SEVER_CC_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        server_cc::Scheduler::GetThis()->schedule(&test_fiber, server_cc::GetThreadId());
    }
}

int main(int argc, char** argv) {
    SEVER_CC_LOG_INFO(g_logger) << "main";
    server_cc::Scheduler sc(2, true, "test");
    sc.start();
    sleep(2);
    SEVER_CC_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    SEVER_CC_LOG_INFO(g_logger) << "over";
    return 0;
}
