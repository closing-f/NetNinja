/*
 * @Author: closing
 * @Date: 2023-06-06 21:38:55
 * @LastEditors: closing
 * @LastEditTime: 2023-06-06 21:40:48
 * @Description: 请填写简介
 */
#include<iostream>
void f(int a){
    std::cout << a << std::endl;
}
#define CALL_WITH_MAX(a,b) f(a>b?a:b)

int main(int argc, char** argv) {
    int a=5,b=0;
    CALL_WITH_MAX(++a,b);
    CALL_WITH_MAX(++a,b+10);
    return 0;
}