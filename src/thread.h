/*
 * @Author: closing
 * @Date: 2023-05-08 08:37:20
 * @LastEditors: closing
 * @LastEditTime: 2023-05-23 09:54:25
 * @Description: 线程类，封装了线程的创建，执行，销毁
 */

#ifndef _THREAD_H
#define _THREAD_H
#include "mutex.h"
#include <string>
namespace server_cc {
class Thread
{

    public:
        typedef std::shared_ptr<Thread> ptr;
        
        Thread(std::function<void()> cb,const std::string& name);
        ~Thread();

        /**
         * @description: 返回线程id
         * @return {*}
         */        
        pid_t getPid(){return m_pid;}

        std::string getName(){return m_name;}

        void join();
        /**
         * @description: 返回当前线程
         * @return {*}
         */        
        static Thread* GetThis();
        /**
         * @description: 返回当前线程名称
         * @return {*}
         */        
        static const std::string& GetName();
        /**
         * @description: 设置当前线程名称
         * @param {string&} name
         * @return {*}
         */        
        static void SetName(const std::string& name);


    private:
        static void* run(void* arg);

        
    private:
        
        pid_t m_pid = -1;//记录线程id
        
        pthread_t m_thread = 0;
        
        std::function<void()> m_cb;//线程执行函数

        std::string m_name; //线程名称
        
        Semaphore m_semaphore;





};
}
#endif 