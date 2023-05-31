/*
 * @Author: closing
 * @Date: 2023-05-27 21:38:15
 * @LastEditors: closing
 * @LastEditTime: 2023-05-27 21:43:37
 * @Description: 请填写简介
 */
#ifndef _SOCKET_STREAM_H
#define _SOCKET_STREAM_H

#include "stream.h"
#include "socket.h"
#include "mutex.h"
#include "iomanager.h"

namespace server_cc{

class SocketStream : public Stream{
public:
    typedef std::shared_ptr<SocketStream> ptr;

    /**
     * @description: 构造函数
     * @param {ptr} sock    socket类
     * @param {bool} owner  //?
     * @return {*}
     */    
    SocketStream(Socket::ptr sock,bool owner = true);

    /**
     * @brief 析构函数
     * @details 如果m_owner=true,则close
     */
    ~SocketStream();


    virtual int read(void* buffer,size_t length) override;

    virtual int read(ByteArray::ptr ba,size_t length) override;

    virtual int write(const void* buffer,size_t length) override;

    virtual int write(ByteArray::ptr ba,size_t length) override;

    virtual void close() override;

    /**
     * @description: 返回socket类
     * @return {*}
     */    
    Socket::ptr getSocket() const { return m_socket;}

    bool isConnected() const;

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();
    std::string getRemoteAddressString();
    std::string getLocalAddressString();

protected:
    Socket::ptr m_socket;
    bool m_owner;//是否需要释放socket

};// class SocketStream



}// namespace server_cc

#endif // !
