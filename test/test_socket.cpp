#include "server_cc/socket.h"
#include "server_cc/server_cc.h"
#include "server_cc/iomanager.h"

static server_cc::Logger::ptr g_looger = SERVER_CC_LOG_ROOT();

void test_socket() {
    //std::vector<server_cc::Address::ptr> addrs;
    //server_cc::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //server_cc::IPAddress::ptr addr;
    //for(auto& i : addrs) {
    //    SERVER_CC_LOG_INFO(g_looger) << i->toString();
    //    addr = std::dynamic_pointer_cast<server_cc::IPAddress>(i);
    //    if(addr) {
    //        break;
    //    }
    //}
    server_cc::IPAddress::ptr addr = server_cc::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr) {
        SERVER_CC_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        SERVER_CC_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    server_cc::Socket::ptr sock = server_cc::Socket::CreateTCP(addr);
    addr->setPort(80);
    SERVER_CC_LOG_INFO(g_looger) << "addr=" << addr->toString();
    if(!sock->connect(addr)) {
        SERVER_CC_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        SERVER_CC_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        SERVER_CC_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        SERVER_CC_LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    SERVER_CC_LOG_INFO(g_looger) << buffs;
}

void test2() {
    server_cc::IPAddress::ptr addr = server_cc::Address::LookupAnyIPAddress("www.baidu.com:80");
    if(addr) {
        SERVER_CC_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        SERVER_CC_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    server_cc::Socket::ptr sock = server_cc::Socket::CreateTCP(addr);
    if(!sock->connect(addr)) {
        SERVER_CC_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        SERVER_CC_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    uint64_t ts = server_cc::GetCurrentUS();
    for(size_t i = 0; i < 10000000000ul; ++i) {
        if(int err = sock->getError()) {
            SERVER_CC_LOG_INFO(g_looger) << "err=" << err << " errstr=" << strerror(err);
            break;
        }

        //struct tcp_info tcp_info;
        //if(!sock->getOption(IPPROTO_TCP, TCP_INFO, tcp_info)) {
        //    SERVER_CC_LOG_INFO(g_looger) << "err";
        //    break;
        //}
        //if(tcp_info.tcpi_state != TCP_ESTABLISHED) {
        //    SERVER_CC_LOG_INFO(g_looger)
        //            << " state=" << (int)tcp_info.tcpi_state;
        //    break;
        //}
        static int batch = 10000000;
        if(i && (i % batch) == 0) {
            uint64_t ts2 = server_cc::GetCurrentUS();
            SERVER_CC_LOG_INFO(g_looger) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " us";
            ts = ts2;
        }
    }
}
//TODO
int main(int argc, char** argv) {
    server_cc::IOManager iom;
    //iom.schedule(&test_socket);
    iom.schedule(&test2);
    return 0;
}
