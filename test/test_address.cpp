/*
 * @Author: closing
 * @Date: 2023-05-22 15:54:05
 * @LastEditors: closing
 * @LastEditTime: 2023-05-31 10:29:58
 * @Description: 请填写简介
 */
#include "src/address.h"
#include "src/logger.h"

server_cc::Logger::ptr g_logger = SERVER_CC_LOG_ROOT();

void test() {
    std::vector<server_cc::Address::ptr> addrs;

    SERVER_CC_LOG_INFO(g_logger) << "begin";
    // bool v = server_cc::Address::Lookup(addrs, "localhost:3080");
    bool v = server_cc::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //bool v = server_cc::Address::Lookup(addrs, "www.server_cc.top", AF_INET);
    SERVER_CC_LOG_INFO(g_logger) << "end";
    if(!v) {
        SERVER_CC_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i) {
        SERVER_CC_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }

    auto addr = server_cc::Address::LookupAny("localhost:4080");
    if(addr) {
        SERVER_CC_LOG_INFO(g_logger) << *addr;
    } else {
        SERVER_CC_LOG_ERROR(g_logger) << "error";
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<server_cc::Address::ptr, uint32_t> > results;

    bool v = server_cc::Address::GetInterfaceAddresses(results);
    if(!v) {
        SERVER_CC_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i: results) {
        SERVER_CC_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }
}

void test_ipv4() {
    //auto addr = server_cc::IPAddress::Create("www.server_cc.top");
    auto addr = server_cc::IPAddress::Create("127.0.0.8");
    if(addr) {
        SERVER_CC_LOG_INFO(g_logger) << addr->toString();
    }
}

int main(int argc, char** argv) {
    test_ipv4();
    // test_iface();
    // test();
    return 0;
}
