/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-13 11:49:54
 * @LastEditors: closing-f fql2018@bupt.edu.cn
 * @LastEditTime: 2023-05-13 22:43:59
 * @FilePath: /server_cc/test_iomanager.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "src/logger.h"
#include "src/thread.h"
#include "src/config.h"
#include "src/mutex.h"
#include "src/fiber.h"
#include "src/utils.h"
// #include "src/scheduler.h"
#include "src/iomanager.h"
#include <yaml-cpp/yaml.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>

server_cc::Logger::ptr g_logger = SEVER_CC_LOG_ROOT();

int sock = 0;

void test_fiber() {
    SEVER_CC_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //server_cc::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "115.239.210.27", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        SEVER_CC_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        server_cc::IOManager::GetThis()->addEvent(sock, server_cc::IOManager::READ, [](){
            SEVER_CC_LOG_INFO(g_logger) << "read callback";
        });
        // SEVER_CC_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        
        // server_cc::IOManager::GetThis()->addEvent(sock, server_cc::IOManager::WRITE, [](){
        //     SEVER_CC_LOG_INFO(g_logger) << "write callback";
        //     //close(sock);
        // });
        server_cc::IOManager::GetThis()->cancelEvent(sock, server_cc::IOManager::READ);
        close(sock);
    } else {
        SEVER_CC_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }

}

void test1() {
    // std::cout << "EPOLLIN=" << EPOLLIN
    //           << " EPOLLOUT=" << EPOLLOUT << std::endl;
    server_cc::IOManager iom(2, false);
    iom.schedule(&test_fiber);
}

// server_cc::Timer::ptr s_timer;
// void test_timer() {
//     server_cc::IOManager iom(2);
//     s_timer = iom.addTimer(1000, [](){
//         static int i = 0;
//         SEVER_CC_LOG_INFO(g_logger) << "hello timer i=" << i;
//         if(++i == 3) {
//             s_timer->reset(2000, true);
//             //s_timer->cancel();
//         }
//     }, true);
// }

int main(int argc, char** argv) {
    test1();
    // test_timer();
    return 0;
}
