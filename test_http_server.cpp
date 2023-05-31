/*
 * @Author: closing
 * @Date: 2023-05-29 16:54:26
 * @LastEditors: closing
 * @LastEditTime: 2023-05-29 17:08:42
 * @Description: 请填写简介
 */
#include "src/http/http_server.h"
#include "src/log.h"

static server_cc::Logger::ptr g_logger = SERVER_CC_LOG_ROOT();

#define XX(...) #__VA_ARGS__


server_cc::IOManager::ptr worker;
void run() {
    g_logger->setLevel(server_cc::LogLevel::INFO);
    //server_cc::http::HttpServer::ptr server(new server_cc::http::HttpServer(true, worker.get(), server_cc::IOManager::GetThis()));
    server_cc::http::HttpServer::ptr server(new server_cc::http::HttpServer(true));
    server_cc::Address::ptr addr = server_cc::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/server_cc/xx", [](server_cc::http::HttpRequest::ptr req
                ,server_cc::http::HttpResponse::ptr rsp
                ,server_cc::http::HttpSession::ptr session) {
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/server_cc/*", [](server_cc::http::HttpRequest::ptr req
                ,server_cc::http::HttpResponse::ptr rsp
                ,server_cc::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });

    sd->addGlobServlet("/server_ccx/*", [](server_cc::http::HttpRequest::ptr req
                ,server_cc::http::HttpResponse::ptr rsp
                ,server_cc::http::HttpSession::ptr session) {
            rsp->setBody(XX(<html>
<head><title>404 Not Found</title></head>
<body>
<center><h1>404 Not Found</h1></center>
<hr><center>nginx/1.16.0</center>
</body>
</html>
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
));
            return 0;
    });

    server->start();
}
//TODO test
int main(int argc, char** argv) {
    server_cc::IOManager iom(1, true, "main");
    worker.reset(new server_cc::IOManager(3, false, "worker"));
    iom.schedule(run);
    return 0;
}
