/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-16 14:54:06
 * @LastEditors: closing
 * @LastEditTime: 2023-06-19 21:28:24
 * @FilePath: /sylar/src/endian.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/**
 * @file endian.h
 * @brief 字节序操作函数(大端/小端)
 * @author sylar.yin
 * @email 564628276@qq.com
 * @date 2019-06-01
 * @copyright Copyright (c) 2019年 sylar.yin All rights reserved (www.sylar.top)
 */
#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#define SERVER_CC_LITTLE_ENDIAN 1
#define SERVER_CC_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

namespace server_cc {

//* 通过enable_if实现模板函数的重载
/**
 * @brief 8字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

/**
 * @brief 4字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

/**
 * @brief 2字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#define SERVER_CC_BYTE_ORDER SERVER_CC_BIG_ENDIAN
#else
#define SERVER_CC_BYTE_ORDER SERVER_CC_LITTLE_ENDIAN
#endif

#if SERVER_CC_BYTE_ORDER == SERVER_CC_BIG_ENDIAN

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template<class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template<class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}
#else

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template<class T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template<class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif

}

#endif
