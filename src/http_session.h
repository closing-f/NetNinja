/*
 * @Author: closing
 * @Date: 2023-05-27 22:00:14
 * @LastEditors: closing
 * @LastEditTime: 2023-05-31 09:51:26
 * @Description: 请填写简介
 */
#ifndef _HTTP_SESSION_H
#define _HTTP_SESSION_H
#include "socket_stream.h"
#include "http/http.h"
namespace server_cc{
namespace http{

/**
 * @description: HttpSession类,服务端使用
 * @return {*}
 */
class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;

    /**
     * @description: 构造函数
     * @param {ptr} sock  socket类
     * @param {bool} owner  
     * @return {*}
     */    
    HttpSession(Socket::ptr sock,bool owner = true);

    /**
     * @description: 接收http请求
     * 
     * @return {*}
     */    
    HttpRequest::ptr recvRequest();

    /**
     * @description:    发送http响应
     * @param {ptr} rsp 响应类
     * @return {*}    返回发送的字节数
     *          >0 发送成功
     *         =0 对方关闭
     *         <0 Socket异常
     */    
    int sendResponse(HttpResponse::ptr rsp);


};// HttpSession

}//namespace http


}//namespace server_cc

#endif // !_H


