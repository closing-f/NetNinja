/*
 * @Author: closing
 * @Date: 2023-05-23 10:51:10
 * @LastEditors: closing
 * @LastEditTime: 2023-05-31 09:42:07
 * @Description: http结构体的封装
 */
#ifndef _HTTP_HTTP_H
#define _HTTP_HTTP_H
#include<memory>
#include<map>
#include<vector>
#include<string>
#include<sstream>
#include<iostream>
#include <boost/lexical_cast.hpp>
#include "http11_parser.h"
#include "httpclient_parser.h"
namespace server_cc{
namespace http{
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)       \

/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \

//? 枚举类的用法
/**
 * @description: http方法的枚举类
 */
enum class HttpMethod {
#define XX(num, name, string) name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
    INVALID_METHOD
};

/**
 * @description: http状态码的枚举类
 */
enum class HttpStatus {
#define XX(code, name, desc) name = code,
    HTTP_STATUS_MAP(XX)
#undef XX
};

/**
 * @description: 将字符串method转换为HttpMethod
 * @param {string&} m 字符串method
 * @return {*}  HttpMethod
 */
HttpMethod StringToHttpMethod(const std::string& m);

 /**
 * @description: 将字符串指针转换成HTTP方法枚举
 * @param m 字符串方法
 * @return HTTP方法枚举
 */
HttpMethod CharsToHttpMethod(const char* m);


std::string HttpMethodToString(const HttpMethod& m);


/**
 * @description: 将HttpStatus转换为字符串
 * @param {HttpStatus} s
 * @return {*}
 */
std::string HttpStatusToString(const HttpStatus& s);


/**
 * @description: 忽略大小写比较仿函数
 */
struct CaseInsensitiveLess {
    bool operator()(const std::string& lhs, const std::string& rhs) const;
};

template<class MapType, class T>
bool checkGetAs(const MapType& m, const std::string& key, T& val, const T& def = T()) {
    auto it = m.find(key);
    if(it == m.end()) {
        val = def;
        return false;
    }
    try {
        val = boost::lexical_cast<T>(it->second);
        return true;
    } catch (...) {
        val = def;
    }
    return false;
}

/**
 * @brief 获取Map中的key值,并转成对应类型
 * @param[in] m Map数据结构
 * @param[in] key 关键字
 * @param[in] def 默认值
 * @return 如果存在且转换成功返回对应的值,否则返回默认值
 */
template<class MapType, class T>
T getAs(const MapType& m, const std::string& key, const T& def = T()) {
    auto it = m.find(key);
    if(it == m.end()) {
        return def;
    }
    try {
        return boost::lexical_cast<T>(it->second);
    } catch (...) {
    }
    return def;
}


class HttpResponse;

class HttpRequest{
public:
    typedef std::shared_ptr<HttpRequest> ptr;

    typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;
    
    
    /**
     * @description: 
     * @param {uint8_t} version 版本号
     * @param {bool} close 是否关闭连接
     * @return {*}
     */    
    HttpRequest(uint8_t version = 0x11, bool close = true);
    
    std::shared_ptr<HttpResponse> createResponse();

    /**
     * @brief 返回HTTP方法
     */
    HttpMethod getMethod() const { return m_method;}

    /**
     * @brief 返回HTTP版本
     */
    uint8_t getVersion() const { return m_version;}

    /**
     * @brief 返回HTTP请求的路径
     */
    const std::string& getPath() const { return m_path;}

    /**
     * @brief 返回HTTP请求的查询参数
     */
    const std::string& getQuery() const { return m_query;}

    /**
     * @brief 返回HTTP请求的消息体
     */
    const std::string& getBody() const { return m_body;}

    /**
     * @brief 返回HTTP请求的消息头MAP
     */
    const MapType& getHeaders() const { return m_headers;}

    /**
     * @brief 返回HTTP请求的参数MAP
     */
    const MapType& getParams() const { return m_params;}

    /**
     * @brief 返回HTTP请求的cookie MAP
     */
    const MapType& getCookies() const { return m_cookies;}

    /**
     * @brief 设置HTTP请求的方法名
     * @param[in] v HTTP请求
     */
    void setMethod(HttpMethod v) { m_method = v;}

    /**
     * @brief 设置HTTP请求的协议版本
     * @param[in] v 协议版本0x11, 0x10
     */
    void setVersion(uint8_t v) { m_version = v;}

    /**
     * @brief 设置HTTP请求的路径
     * @param[in] v 请求路径
     */
    void setPath(const std::string& v) { m_path = v;}

    /**
     * @brief 设置HTTP请求的查询参数
     * @param[in] v 查询参数
     */
    void setQuery(const std::string& v) { m_query = v;}

    /**
     * @brief 设置HTTP请求的Fragment
     * @param[in] v fragment
     */
    void setFragment(const std::string& v) { m_fragment = v;}

    /**
     * @brief 设置HTTP请求的消息体
     * @param[in] v 消息体
     */
    void setBody(const std::string& v) { m_body = v;}

    /**
     * @brief 是否自动关闭
     */
    bool isClose() const { return m_close;}

    /**
     * @brief 设置是否自动关闭
     */
    void setClose(bool v) { m_close = v;}

    /**
     * @brief 是否websocket
     */
    bool isWebsocket() const { return m_websocket;}

    /**
     * @brief 设置是否websocket
     */
    void setWebsocket(bool v) { m_websocket = v;}

    /**
     * @brief 设置HTTP请求的头部MAP
     * @param[in] v map
     */
    void setHeaders(const MapType& v) { m_headers = v;}

    /**
     * @brief 设置HTTP请求的参数MAP
     * @param[in] v map
     */
    void setParams(const MapType& v) { m_params = v;}

    /**
     * @brief 设置HTTP请求的Cookie MAP
     * @param[in] v map
     */
    void setCookies(const MapType& v) { m_cookies = v;}

    /**
     * @brief 获取HTTP请求的头部参数
     * @param[in] key 关键字
     * @param[in] def 默认值
     * @return 如果存在则返回对应值,否则返回默认值
     */
    std::string getHeader(const std::string& key, const std::string& def = "") ;

    /**
     * @brief 获取HTTP请求的请求参数
     * @param[in] key 关键字
     * @param[in] def 默认值
     * @return 如果存在则返回对应值,否则返回默认值
     */
    std::string getParam(const std::string& key, const std::string& def = "");

    /**
     * @brief 获取HTTP请求的Cookie参数
     * @param[in] key 关键字
     * @param[in] def 默认值
     * @return 如果存在则返回对应值,否则返回默认值
     */
    std::string getCookie(const std::string& key, const std::string& def = "");

    
    /**
     * @brief 设置HTTP请求的头部参数
     * @param[in] key 关键字
     * @param[in] val 值
     */
    void setHeader(const std::string& key, const std::string& val);

    /**
     * @brief 设置HTTP请求的请求参数
     * @param[in] key 关键字
     * @param[in] val 值
     */

    void setParam(const std::string& key, const std::string& val);
    /**
     * @brief 设置HTTP请求的Cookie参数
     * @param[in] key 关键字
     * @param[in] val 值
     */
    void setCookie(const std::string& key, const std::string& val);

    /**
     * @brief 删除HTTP请求的头部参数
     * @param[in] key 关键字
     */
    void delHeader(const std::string& key);

    /**
     * @brief 删除HTTP请求的请求参数
     * @param[in] key 关键字
     */
    void delParam(const std::string& key);

    /**
     * @brief 删除HTTP请求的Cookie参数
     * @param[in] key 关键字
     */
    void delCookie(const std::string& key);

    /**
     * @brief 判断HTTP请求的头部参数是否存在
     * @param[in] key 关键字
     * @param[out] val 如果存在,val非空则赋值
     * @return 是否存在
     */
    bool hasHeader(const std::string& key, std::string* val = nullptr);

    /**
     * @brief 判断HTTP请求的请求参数是否存在
     * @param[in] key 关键字
     * @param[out] val 如果存在,val非空则赋值
     * @return 是否存在
     */
    
    bool hasParam(const std::string& key, std::string* val = nullptr);

    /**
     * @brief 判断HTTP请求的Cookie参数是否存在
     * @param[in] key 关键字
     * @param[out] val 如果存在,val非空则赋值
     * @return 是否存在
     */
    bool hasCookie(const std::string& key, std::string* val = nullptr);

    /**
     * @description: 获取HTTP请求的头部参数
     * @return {*}
     */    
    template <class T>
    bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
        return checkGetAs(m_headers, key, val, def);
    }

    
    template <class T>
    T getHeaderAs(const std::string& key, const T& def = T()) {
        return getAs(m_headers, key, def);
    }

    template <class T>
    bool checkGetParamAs(const std::string& key, T& val, const T& def = T()) {
        return checkGetAs(m_params, key, val, def);
    }
    
    template <class T>
    T getParamAs(const std::string& key, const T& def = T()) {
        return getAs(m_headers, key, def);
    }

    template <class T>
    bool checkGetCookieAs(const std::string& key, T& val, const T& def = T()) {
        return checkGetAs(m_headers, key, val, def);
    }

    
    template <class T>
    T getCookieAs(const std::string& key, const T& def = T()) {
        return getAs(m_headers, key, def);
    }

    /**
     * @brief 将HTTP请求转成字符串
     */
    std::ostream& dump(std::ostream& os);

private:

   

    // void init();
    // void initParam();
    // void initQueryParam();
    // void initBodyParam();
    // void initCookies();
private:
    /// HTTP方法
    HttpMethod m_method;
    /// HTTP版本
    uint8_t m_version;
    /// 是否自动关闭
    bool m_close;
    /// 是否为websocket
    bool m_websocket;

    uint8_t m_parserParamFlag;
    /// 请求路径
    std::string m_path;
    /// 请求参数
    std::string m_query;
    /// 请求fragment
    std::string m_fragment;
    /// 请求消息体
    std::string m_body;
    /// 请求头部MAP
    MapType m_headers;
    /// 请求参数MAP
    MapType m_params;
    /// 请求Cookie MAP
    MapType m_cookies;
};

class HttpResponse{
public:
    typedef std::shared_ptr<HttpResponse> ptr;
    typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;
    HttpResponse(uint8_t version = 0x11, bool close = true);

    HttpStatus getStatus() const { return m_status;}
    uint8_t getVersion() const { return m_version;}
    const std::string& getBody() const { return m_body;}
    const std::string& getReason() const { return m_reason;}
    const MapType& getHeaders() const { return m_headers;}

    void setStatus(HttpStatus v) { m_status = v;}
    void setVersion(uint8_t v) { m_version = v;}
    void setBody(const std::string& v) { m_body = v;}
    void setReason(const std::string& v) { m_reason = v;}
    void setHeaders(const MapType& v) { m_headers = v;}
    
    bool isClose() const { return m_close;}
    void setClose(bool v) { m_close = v;}

    std::string getHeader(const std::string& key, const std::string& def = "") const;

    /**
     * @brief 设置响应头部参数
     * @param[in] key 关键字
     * @param[in] val 值
     */
    void setHeader(const std::string& key, const std::string& val);

    /**
     * @brief 删除响应头部参数
     * @param[in] key 关键字
     */
    void delHeader(const std::string& key);

    std::ostream& dump(std::ostream& os);
    //? toString const会报错
    std::string toString();
    template <class T>
    bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
        return checkGetAs(m_headers, key, val, def);
    }

    template <class T>
    T getHeaderAs(const std::string& key, const T& def = T()) {
        return getAs(m_headers, key, def);
    }
private:
    /// HTTP版本
    uint8_t m_version;
    /// 是否自动关闭
    bool m_close;
    /// 是否为websocket
    bool m_websocket;
    /// 状态码
    HttpStatus m_status;
    /// 状态描述
    std::string m_reason;
    /// 响应头部MAP
    MapType m_headers;
    /// 响应正文
    std::string m_body;

    std::vector<std::string> m_cookies;
};
//? telnet模拟发送http报文

/**
 * @brief 流式输出HttpRequest
 * @param[in, out] os 输出流
 * @param[in] req HTTP请求
 * @return 输出流
 */
std::ostream& operator<<(std::ostream& os, HttpRequest& req);

/**
 * @brief 流式输出HttpResponse
 * @param[in, out] os 输出流
 * @param[in] rsp HTTP响应
 * @return 输出流
 */
std::ostream& operator<<(std::ostream& os, HttpResponse& rsp);

} // namespace http
} // namespace server_cc















#endif
