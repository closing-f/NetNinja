/*
 * @Author: closing
 * @Date: 2023-05-23 20:27:59
 * @LastEditors: closing
 * @LastEditTime: 2023-05-23 20:57:32
 * @Description: 请填写简介
 */
#ifndef _http11_common_h
#define _http11_common_h

#include <sys/types.h>

typedef void (*element_cb)(void *data, const char *at, size_t length);
typedef void (*field_cb)(void *data, const char *field, size_t flen, const char *value, size_t vlen);

#endif