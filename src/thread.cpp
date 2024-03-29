/*
 * @Author: closing
 * @Date: 2023-05-08 08:37:29
 * @LastEditors: closing
 * @LastEditTime: 2023-05-23 09:58:16
 * @Description: 请填写简介
 */
#include"thread.h"
#include "logger.h"
namespace server_cc{

static thread_local Thread* t_thread=nullptr;
static thread_local std::string t_thread_name="UNKNOWN";
static Logger::ptr g_logger = SERVER_CC_LOG_NAME("system");

Thread::Thread(std::function<void()> cb,const std::string&name):m_cb(cb),m_name(name){
    if(name.empty()){
        m_name = "UNKNOWN";
    }
    int rt = pthread_create(&m_thread,NULL,&Thread::run,this);
    if (rt){
        throw std::logic_error("pthread_create error");
    }
    m_semaphore.wait();//等待线程运行起来

}
void* Thread::run(void*arg){

    Thread* thread=(Thread*)arg;
    t_thread=thread;
    t_thread_name=thread->getName();
    thread->m_pid=server_cc::GetThreadId();
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
    
    //? 减少引用次数
    std::function<void()> cb;
    cb.swap(thread->m_cb);
    
    thread->m_semaphore.notify();
    cb();
    return 0;
}

Thread* Thread::GetThis(){
    return t_thread;
}
const std::string& Thread::GetName() {
    return t_thread_name;
}

void Thread::SetName(const std::string& name) {
    if(name.empty()) {
        return;
    }
    if(t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}
Thread::~Thread() {
    if(m_thread) {
        pthread_detach(m_thread);
    }
}
void Thread::join() {
    if(m_thread) {
        int rt = pthread_join(m_thread, nullptr);
        if(rt) {
            SERVER_CC_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                << " name=" << m_name;
            throw std::logic_error("pthread_join error");
        }
        
        m_thread = 0;
    }
}
}