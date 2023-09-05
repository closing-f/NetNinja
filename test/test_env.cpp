/*
 * @Author: closing
 * @Date: 2023-07-10 09:56:49
 * @LastEditors: closing
 * @LastEditTime: 2023-07-10 09:57:01
 * @Description: 请填写简介
 */
#include "src/env.h"
#include <unistd.h>
#include <iostream>
#include <fstream>

struct A {
    A() {
        std::ifstream ifs("/proc/" + std::to_string(getpid()) + "/cmdline", std::ios::binary);
        std::string content;
        content.resize(4096);

        ifs.read(&content[0], content.size());
        content.resize(ifs.gcount());

        for(size_t i = 0; i < content.size(); ++i) {
            std::cout << i << " - " << content[i] << " - " << (int)content[i] << std::endl;
        }
    }
};

A a;

int main(int argc, char** argv) {
    std::cout << "argc=" << argc << std::endl;
    server_cc::EnvMgr::GetInstance().addHelp("s", "start with the terminal");
    server_cc::EnvMgr::GetInstance().addHelp("d", "run as daemon");
    server_cc::EnvMgr::GetInstance().addHelp("p", "print help");
    if(!server_cc::EnvMgr::GetInstance().init(argc, argv)) {
        server_cc::EnvMgr::GetInstance().printHelp();
        return 0;
    }

    std::cout << "exe=" << server_cc::EnvMgr::GetInstance().getExe() << std::endl;
    std::cout << "cwd=" << server_cc::EnvMgr::GetInstance().getCwd() << std::endl;

    std::cout << "path=" << server_cc::EnvMgr::GetInstance().getEnv("PATH", "xxx") << std::endl;
    std::cout << "test=" << server_cc::EnvMgr::GetInstance().getEnv("TEST", "") << std::endl;
    std::cout << "set env " << server_cc::EnvMgr::GetInstance().setEnv("TEST", "yy") << std::endl;
    std::cout << "test=" << server_cc::EnvMgr::GetInstance().getEnv("TEST", "") << std::endl;
    if(server_cc::EnvMgr::GetInstance().has("p")) {
        server_cc::EnvMgr::GetInstance().printHelp();
    }
    return 0;
}
