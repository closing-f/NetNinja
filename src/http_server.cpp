/*
 * @Author: closing
 * @Date: 2023-05-28 09:55:19
 * @LastEditors: closing
 * @LastEditTime: 2023-05-31 09:57:55
 * @Description: 请填写简介
 */
#include "http_server.h"
#include "logger.h"

namespace server_cc {
namespace http {

static server_cc::Logger::ptr g_logger = SERVER_CC_LOG_NAME("system");

HttpServer::HttpServer(bool keepalive
               ,server_cc::IOManager* worker
               ,server_cc::IOManager* io_worker
               ,server_cc::IOManager* accept_worker)
    :TcpServer(worker, io_worker, accept_worker)
    ,m_isKeepalive(keepalive) {
    m_dispatch.reset(new ServletDispatch);

    m_type = "http";
    // m_dispatch->addServlet("/_/status", Servlet::ptr(new StatusServlet));
    // m_dispatch->addServlet("/_/config", Servlet::ptr(new ConfigServlet));
}

void HttpServer::setName(const std::string& v) {
    TcpServer::setName(v);
    m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

void HttpServer::handleClient(Socket::ptr client) {
    SERVER_CC_LOG_DEBUG(g_logger) << "handleClient " << *client;
    HttpSession::ptr session(new HttpSession(client));
    do {
        auto req = session->recvRequest();
        if(!req) {
            SERVER_CC_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                << errno << " errstr=" << strerror(errno)
                << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
            break;
        }

        HttpResponse::ptr rsp(new HttpResponse(req->getVersion()
                            ,req->isClose() || !m_isKeepalive));
        rsp->setHeader("Server", getName());
        m_dispatch->handle(req, rsp, session);
        session->sendResponse(rsp);

        if(!m_isKeepalive || req->isClose()) {
            break;
        }
    } while(true);
    session->close();
}

}
}
