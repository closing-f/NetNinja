/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-24 21:31:05
 * @LastEditors: closing
 * @LastEditTime: 2023-05-31 10:01:19
 * @FilePath: /sylar/src/tcpserver.h
 * @Description: TCP server
 */
#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <memory>
#include "iomanager.h"
#include "socket.h"
#include <functional>
namespace server_cc { 

class TcpServer : public std::enable_shared_from_this<TcpServer>,Noncopyable {
public:
    typedef std::shared_ptr<TcpServer> ptr;

    /**
     * @description: 构造函数
     * @param {IOManager*} worker    用于处理新连接的调度器
     * @param {IOManager*} io_worker    用于处理读写事件的调度器
     * @param {IOManager*} accept_worker    用于处理接收连接的调度器
     * @return {*}
     */    
    TcpServer(server_cc::IOManager* worker = server_cc::IOManager::GetThis(),
            server_cc::IOManager* acceptor = server_cc::IOManager::GetThis(),
            server_cc::IOManager* io_woker = server_cc::IOManager::GetThis());
    virtual ~TcpServer();

    
    /**
     * @description: 绑定地址,//bind函数中包含了listen函数
     * @param {ptr} addr
     * @param {bool} ssl
     * @return {*}
     */    
    virtual bool bind(server_cc::Address::ptr addr,bool ssl = false);
    virtual bool bind(const std::vector<server_cc::Address::ptr>& addrs,
                        std::vector<Address::ptr>& fails,
                        bool ssl = false);
    /**
     * @description: 启动服务，开始接收连接（在bind之后调用）
     * @return {*}
     */    
    virtual bool start();
    virtual void stop();
    //? uint64_t等价大小
    uint64_t getRecvTimeout() const { return m_recvTimeout;}

    /**
     * @brief 返回服务器名称
     */
    std::string getName() const { return m_name;}

    /**
     * @brief 设置读取超时时间(毫秒)
     */
    void setRecvTimeout(uint64_t v) { m_recvTimeout = v;}

    virtual void setName(const std::string& v) { m_name = v;}
    bool isStop() const { return m_isStop;}

    virtual std::string toString(const std::string& prefix = "");

    std::vector<Socket::ptr> getSocks() const { return m_socks;}

protected:
    virtual void handleClient(Socket::ptr client);//处理客户端连接，每accept一个连接，就会创建一个协程
    
    /**
     * @description:    开始接收连接
     * @param {ptr} sock
     * @return {*}
     */    
    virtual void startAccept(Socket::ptr sock);//

protected:
    std::vector<Socket::ptr> m_socks;//监听多个地址
    /// 新连接的Socket工作的调度器
    IOManager* m_worker;
    IOManager* m_ioWorker;
    /// 服务器Socket接收连接的调度器
    IOManager* m_acceptWorker;
    uint64_t m_recvTimeout;
    /// 服务器名称
    std::string m_name;
    /// 服务器类型
    std::string m_type = "tcp";
    /// 服务是否停止
    bool m_isStop;

    bool m_ssl = false;



};



}// namespace server_cc

#endif // TCPSERVER_H