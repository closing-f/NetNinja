#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <memory>
#include "iomanager.h"
#include "socket.h"
#include <functional>
namespace server_cc { 

class TcpServer : public std::enable_shared_from_this<TcpServer> {
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(server_cc::IOManager* worker = server_cc::IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(server_cc::Address::ptr addr);
    virtual bool bind(const std::vector<server_cc::Address::ptr>& addrs);
    virtual bool start();
    virtual void stop();
    //? uint64_t等价大小
    uint64_t getReadTimeout() const { return m_readTimeout;}
    std::string getName() const { return m_name;}
    void setReadTimeout(uint64_t v) { m_readTimeout = v;}
    void setName(const std::string& v) { m_name = v;}

    bool isStop() const { return m_isStop;}

protected:
    virtual void handleClient(Socket::ptr client);//处理客户端连接，每accept一个连接，就会创建一个协程
    virtual void startAccept(Socket::ptr sock);
    virtual void handleClient(Socket::ptr client) = 0;

private:
    std::vector<Socket::ptr> m_socks;//监听多个地址
    IOManager* m_worker;
    uint64_t m_readTimeout;//超时时间
    std::string m_name;
    bool m_isStop;//描述server是否停止



};



}// namespace server_cc

#endif // TCPSERVER_H