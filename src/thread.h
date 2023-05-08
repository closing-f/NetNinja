/*
*@thread.h
*/
#ifndef _THREAD_H
#define _THREAD_H
#include "mutex.h"
#include "logger.h"
#include <string>
namespace server_cc{
class Thread
{

    public:
        typedef std::shared_ptr<Thread> ptr;
        
        Thread(std::function<void()> cb,const std::string& name);
        ~Thread();
        pid_t getPid(){return m_pid;}

        std::string getName(){return m_name;}

        void join();

        static Thread* GetThis();

        static const std::string& GetName();

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