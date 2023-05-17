/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-16 11:15:51
 * @LastEditors: closing-f fql2018@bupt.edu.cn
 * @LastEditTime: 2023-05-16 11:46:02
 * @FilePath: /server_cc/test_hook.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "src/hook.h"
#include "src/logger.h"
#include "src/iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>


server_cc::Logger::ptr g_logger = SEVER_CC_LOG_ROOT();

void test_sleep() {
    server_cc::IOManager iom(1);
    iom.schedule([](){
        sleep(2);
        SEVER_CC_LOG_INFO(g_logger) << "sleep 2";
    });

    iom.schedule([](){
        sleep(3);
        SEVER_CC_LOG_INFO(g_logger) << "sleep 3";
    });
    SEVER_CC_LOG_INFO(g_logger) << "test_sleep";
}

void test_sock() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "39.156.66.14", &addr.sin_addr.s_addr);

    SEVER_CC_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    SEVER_CC_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

    if(rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    SEVER_CC_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    SEVER_CC_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    buff.resize(rt);
    SEVER_CC_LOG_INFO(g_logger) << buff;
}

int main(int argc, char** argv) {
    //test_sleep();
    server_cc::set_hook_enable(true);
    server_cc::IOManager iom;
    iom.schedule(test_sock);
    return 0;
}
