/*
 * @Author: closing
 * @Date: 2023-05-27 22:46:35
 * @LastEditors: closing
 * @LastEditTime: 2023-07-10 09:37:24
 * @Description: 请填写简介
 */
#ifndef _SERVER_CC_HTTP_SERVER_H
#define _SERVER_CC_HTTP_SERVER_H

#include "tcpserver.h"
#include "http/http_session.h"
#include "http/http_servlet.h"

namespace server_cc{
namespace http{

class HttpServer : public TcpServer{
public:
    typedef std::shared_ptr<HttpServer> ptr;

    /**
     * @description: 构造函数
     * @param {bool} keepalive   是否支持长连接
     * @param {server_cc::IOManager*} worker    工作调度器
     * @param {server_cc::IOManager*} io_worker    读写调度器
     * @param {server_cc::IOManager*} accept_worker    接收连接调度器
     * @return {*}
     */    
    HttpServer(bool keepalive = false
            ,server_cc::IOManager* worker = server_cc::IOManager::GetThis()
            ,server_cc::IOManager* io_worker = server_cc::IOManager::GetThis()
            ,server_cc::IOManager* accept_worker = server_cc::IOManager::GetThis());

    //TODO 
    /**
     * @description: 获取ServletDispatch
     * @return {*}
     */    
    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}

    /**
     * @brief 设置ServletDispatch
     */
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}

    virtual void setName(const std::string& v) override;
protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    /// 是否支持长连接
    bool m_isKeepalive;
    /// Servlet分发器
    ServletDispatch::ptr m_dispatch;



};//HttpServer




}//namespace http
}//namespace server_cc
#endif // !
