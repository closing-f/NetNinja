#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include "socket_stream.h"
#include "http/http.h"
#include "uri.h"
#include "thread.h"
#include <list>
namespace server_cc{
namespace http{


/**
 * @description: http请求结果
 * @return {*}
 */
struct HttpResult{
    typedef std::shared_ptr<HttpResult> ptr;

    /**
     * @description: 错误码
     * @return {*}
     */    
    enum class Error{
        OK = 0,
        INVALID_URL = 1,
        INVALID_HOST = 2,
        CONNECT_FAIL = 3,
        SEND_CLOSE_BY_PEER = 4,
        SEND_SOCKET_ERROR = 5,
        TIMEOUT = 6,
        CREATE_SOCKET_ERROR = 7,
        POOL_GET_CONNECTION = 8,
        POOL_INVALID_CONNECTION = 9,
    };

    /**
     * @description: 构造函数
     * @param {int32_t} result 错误码
     * @param {HttpResponse::ptr} response 响应结构体
     * @param {const std::string&} error 错误描述
     * @return {*}
     */
    HttpResult(int32_t _result,HttpResponse::ptr _response,const std::string& _error)
        :result(_result)
        ,response(_response)
        ,error(_error){}
    int result;
    //响应结构体
    HttpResponse::ptr response;
    
    std::string error;

    std::string toString() const;
};

class HttpConnectionPool;

class HttpConnection : public SocketStream{
friend class HttpConnectionPool;
public:
    typedef std::shared_ptr<HttpConnection> ptr;


    /**
     * @description: 发送HTTP请求
     * @param {HttpMethod} method 请求方法
     * @param {const std::string&} url 请求地址
     * @param {uint64_t} timeout_ms 超时时间(毫秒)
     * @param {const std::map<std::string, std::string>&} headers HTTP请求头部参数
     * @param {const std::string&} body 请求消息体
     * @return {*}
     */    
    static HttpResult::ptr DoRequest(HttpMethod method
                            , const std::string& url
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    /**
     * @description: 发送HTTP请求
     * @param {HttpMethod} method 请求方法
     * @param {Uri::ptr} uri URI结构体
     * @param {uint64_t} timeout_ms 超时时间(毫秒)
     * @param {const std::map<std::string, std::string>&} headers HTTP请求头部参数
     * @param {const std::string&} body 请求消息体
     * @return {*}
     */    
    static HttpResult::ptr DoRequest(HttpMethod method
                            , Uri::ptr uri
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    /**
     * @description: 发送HTTP请求
     * @param {HttpRequest::ptr} req 请求结构体
     * @param {Uri::ptr} uri URI结构体
     * @param {uint64_t} timeout_ms 超时时间(毫秒)
     * @return {*}
     */    
    static HttpResult::ptr DoRequest(HttpRequest::ptr req
                            , Uri::ptr uri
                            , uint64_t timeout_ms);

    /**
     * @description: 构造函数
     * @param {ptr} sock    socket
     * @param {bool} owner  是否掌握所有权
     * @return {*}
     */    
    HttpConnection(Socket::ptr sock,bool owner = true);

    ~HttpConnection();

    HttpResponse::ptr recvResponse();


    /**
     * @description: 发送请求
     * @param {ptr} req 请求
     * @return {*}
     */    
    int sendRequest(HttpRequest::ptr req);
private:
    uint64_t m_createTime = 0;//创建时间
    uint64_t m_request = 0;//请求次数
};

class HttpConnectionPool{
public:
    typedef std::shared_ptr<HttpConnectionPool> ptr;
    typedef Mutex MutexType;

    static HttpConnectionPool::ptr Create(const std::string& uri
                                        ,const std::string& vhost
                                        ,uint32_t max_size
                                        ,uint32_t max_alive_time
                                        ,uint32_t max_request);

    HttpConnectionPool(const std::string& host
                    ,const std::string& vhost
                    ,uint32_t port
                    ,bool is_https
                    ,uint32_t max_size
                    ,uint32_t max_alive_time
                    ,uint32_t max_request);

    HttpConnection::ptr getConnection();

    
    HttpResult::ptr doRequest(HttpMethod method
                            , const std::string& url
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    /**
     * @brief 发送HTTP请求
     * @param[in] method 请求类型
     * @param[in] uri URI结构体
     * @param[in] timeout_ms 超时时间(毫秒)
     * @param[in] headers HTTP请求头部参数
     * @param[in] body 请求消息体
     * @return 返回HTTP结果结构体
     */
    HttpResult::ptr doRequest(HttpMethod method
                            , Uri::ptr uri
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = "");

    /**
     * @brief 发送HTTP请求
     * @param[in] req 请求结构体
     * @param[in] timeout_ms 超时时间(毫秒)
     * @return 返回HTTP结果结构体
     */
    HttpResult::ptr doRequest(HttpRequest::ptr req
                            , uint64_t timeout_ms);
private:
    static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);
private:
    std::string m_host;
    std::string m_vhost;
    uint32_t m_port;
    uint32_t m_maxSize;//最大连接数
    uint32_t m_maxAliveTime;//最大连接时间
    uint32_t m_maxRequest;//最大请求数
    bool m_isHttps;

    MutexType m_mutex;
    std::list<HttpConnection*> m_conns;
    std::atomic<int32_t> m_total = {0};//当前总连接数
};

} // namespace http
} // namespace server_cc

#endif // !

