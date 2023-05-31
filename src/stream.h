/*
 * @Author: closing
 * @Date: 2023-05-25 15:16:46
 * @LastEditors: closing
 * @LastEditTime: 2023-05-31 09:19:48
 * @Description: 请填写简介
 */
#ifndef STREAM_H
#define STREAM_H

#include <memory>
#include <string>
#include "bytearray.h"
namespace server_cc{
class Stream{
public:
    typedef std::shared_ptr<Stream> ptr;

    virtual ~Stream(){}

    /**
     * @description: 读取固定长度的数据到buffer中
     * @param {void*} buffer    读取的数据存放的位置
     * @param {size_t} length   读取的数据长度
     * @return {*}
     */    
    virtual int read(void* buffer,size_t length) = 0;

    /**
     * @description: 读取固定长度的数据到ByteArray中
     * @param {ptr} ba  读取的数据存放的位置
     * @param {size_t} length   读取的数据长度
     * @return {*}
     */    
    virtual int read(ByteArray::ptr ba,size_t length) = 0;

    /**
     * @description: 写入固定长度的数据到buffer中
     * @param {void*} buffer    写入的数据存放的位置
     * @param {size_t} length  写入的数据长度
     * @return {*}
     */    
    virtual int readFixSize(void* buffer,size_t length);

    /**
     * @description: 写入固定长度的数据到ByteArray中
     * @param {ptr} ba  写入的数据存放的位置
     * @param {size_t} length   写入的数据长度
     * @return {*}
     */    
    virtual int readFixSize(ByteArray::ptr ba,size_t length);

    /**
     * @description: 将buffer中的数据发送出去
     * @param {void*} buffer    写入的数据存放的位置
     * @param {size_t} length   写入的数据长度
     * @return {*}
     */    
    virtual int write(const void* buffer,size_t length) = 0;

    /**
     * @description: 将buffer中的数据发送出去
     * @param {ptr} ba  写入的数据存放的位置
     * @param {size_t} length   写入的数据长度
     * @return {*}
     */    
    virtual int write(ByteArray::ptr ba,size_t length) = 0;

    /**
     * @description: 将buffer中的数据发送出去
     * @param {void*} buffer    写入的数据存放的位置
     * @param {size_t} length   写入的数据长度
     * @return {*}
     */    
    virtual int writeFixSize(const void* buffer,size_t length);

    /**
     * @description:    将buffer中的数据发送出去
     * @param {ptr} ba  写入的数据存放的位置
     * @param {size_t} length   写入的数据长度
     * @return {*}
     */    
    virtual int writeFixSize(ByteArray::ptr ba,size_t length);
    
    /**
     * @description:    关闭流
     * @return {*}  void
     */    
    virtual void close() = 0;
};
}
#endif // !STREAM_H
