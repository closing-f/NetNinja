#include "src/logger.h"
#include "src/thread.h"
#include "src/config.h"
#include "src/mutex.h"
#include <yaml-cpp/yaml.h>
server_cc::Logger::ptr g_logger = SEVER_CC_LOG_ROOT();
int count = 0;
server_cc::Mutex mutex;

void fun1() {
    SEVER_CC_LOG_INFO(g_logger) << "name: " << server_cc::Thread::GetName()
                             << " this.name: " << server_cc::Thread::GetThis()->getName()
                             << " id: " << server_cc::GetThreadId()
                             << " this.id: " << server_cc::Thread::GetThis()->getPid();
    
    for(int i = 0; i < 100; ++i) {
        //server_cc::RWMutex::WriteLock lock(s_mutex); 
        server_cc::Mutex::Lock lock(mutex);
        ++count;
    }
}

int main(int argc, char** argv) {
    SEVER_CC_LOG_INFO(g_logger) << "thread test begin";
    YAML::Node root = YAML::LoadFile("/home/fql/Project/C++/sylar/log.yml");
    server_cc::Config::LoadFromYaml(root);

    std::vector<server_cc::Thread::ptr> thrs;
    for(int i = 0; i < 5; ++i) {
        server_cc::Thread::ptr thr(new server_cc::Thread(&fun1, "name_" + std::to_string(i * 2)));
        //server_cc::Thread::ptr thr2(new server_cc::Thread(&fun3, "name_" + std::to_string(i * 2 + 1)));
        thrs.push_back(thr);
        //thrs.push_back(thr2);
    }

    for(size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }
    SEVER_CC_LOG_INFO(g_logger) << "thread test end";
    SEVER_CC_LOG_INFO(g_logger) << "count=" << count;

    return 0;
}