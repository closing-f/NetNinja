/*
 * @Author: closing
 * @Date: 2023-05-24 21:50:16
 * @LastEditors: closing
 * @LastEditTime: 2023-05-31 21:16:30
 * @Description: 请填写简介
 */
#include "tcpserver.h"
#include "config.h"

server_cc::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout = server_cc::Config::Lookup("tcp_server.read_timeout", (uint64_t)(60*1000*2), "tcp server read timeout");
static server_cc::Logger::ptr g_logger = SERVER_CC_LOG_NAME("system");
namespace server_cc{

TcpServer::TcpServer(server_cc::IOManager* worker,
            server_cc::IOManager* io_worker,
            server_cc::IOManager* accept_worker)
    :m_worker(worker)
    ,m_ioWorker(io_worker)
    ,m_acceptWorker(accept_worker)
    ,m_recvTimeout(g_tcp_server_read_timeout->getValue())
    ,m_name("server_cc/1.0.0")
    ,m_isStop(true){//在start的时候设置为false

}

TcpServer::~TcpServer(){
    for(auto& sock : m_socks){
        sock->close();
    }
    m_socks.clear();
}

bool TcpServer::bind(server_cc::Address::ptr addr,bool ssl){
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs,fails,ssl);
}
//? void 指针的用法
bool TcpServer::bind(const std::vector<Address::ptr>& addrs
    ,std::vector<server_cc::Address::ptr>& fails,bool ssl){

    for(auto& addr : addrs){
        Socket::ptr sock = Socket::CreateTCP(addr);
        if(!sock->bind(addr)){
            SERVER_CC_LOG_ERROR(g_logger) << "bind fail errno = " << errno
                << " errstr = " << strerror(errno)
                << " addr = " << addr->toString();
            fails.push_back(addr);
            continue;
        }
        if(!sock->listen()){
            SERVER_CC_LOG_ERROR(g_logger) << "listen fail errno = " << errno
                << " errstr = " << strerror(errno)
                << " addr = " << addr->toString();
            fails.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }
    if(!fails.empty()){
        m_socks.clear();
        return false;
    }
   
    for(auto& sock : m_socks){
        SERVER_CC_LOG_INFO(g_logger) << "server bind success: " << *sock;
    }
    return true;
}

void TcpServer::startAccept(Socket::ptr sock){
    while(!m_isStop){
        Socket::ptr client = sock->accept();
        SERVER_CC_LOG_INFO(g_logger) << "accept !!";
        if(client){
            m_worker->schedule(std::bind(&TcpServer::handleClient,shared_from_this(),
                    client));//传智能指针，保证在handleClient结束前TcpServer不会被释放掉
        }else{
            SERVER_CC_LOG_ERROR(g_logger) << "accept errno=" << errno <<" errstr="
            <<strerror(errno);
            
        }
    }
}

bool TcpServer::start(){
    if(!m_isStop){
        return true;
    }
    m_isStop = false;
    for(auto& sock : m_socks){
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
                                shared_from_this(),sock));

    }
    return true;
}

void TcpServer::stop(){
    m_isStop=true;
    auto self = shared_from_this();
    m_acceptWorker->schedule([this,self](){
        for(auto& sock: m_socks){
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
}
void TcpServer::handleClient(Socket::ptr client) {
    SERVER_CC_LOG_INFO(g_logger) << "handleClient: " << *client;
}

std::string TcpServer::toString(const std::string& prefix){
    std::stringstream ss;
    ss << prefix << "[type = " << m_type 
        << " name = " << m_name
        << " worker = " << (m_worker ? m_worker->getName() : "")
        << " accept_worker = " << (m_acceptWorker ? m_acceptWorker->getName() : "")
        << " io_worker = " << (m_ioWorker ? m_ioWorker->getName() : "")
        << " recv_timeout = " << m_recvTimeout << "]" << std::endl;
    
    std::string pfx = prefix.empty() ? "    " : prefix;
    for(auto& i : m_socks) {
        //? << *i 为什么可以直接输出,因为在socket.h中重载了<<
        ss << pfx << pfx << *i << std::endl;
    }
    return ss.str();
}

}// namespace server_cc