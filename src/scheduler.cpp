/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-11 11:35:09
 * @LastEditors: closing-f fql2018@bupt.edu.cn
 * @LastEditTime: 2023-05-12 21:06:56
 * @FilePath: /sylar/src/scheduler.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "scheduler.h"
#include "utils.h"
#include "logger.h"
namespace server_cc{

static Logger::ptr g_logger = SEVER_CC_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;// 当前线程的调度器
static thread_local Fiber* t_scheduler_fiber = nullptr;// 当前线程的协程
Scheduler::Scheduler(size_t thread_num, bool use_caller, const std::string& name):m_name(name){
    
    if(use_caller){
        // 当前线程是主线程,创建主协程
        Fiber::GetThis();
        --thread_num;
        SEVER_ASSERT(GetThis() == nullptr);
        
        setThis();// 设置当前线程的调度器
        
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        server_cc::Thread::SetName(m_name);

        t_scheduler_fiber = m_rootFiber.get();
        m_rootThread = server_cc::GetThreadId();
        m_threadIds.push_back(m_rootThread);

    } 
    //? 为何不是主线程
     else {
        m_rootThread = -1;
    }

    m_threadNum = thread_num;
    
    
}

Scheduler::~Scheduler(){
    SEVER_ASSERT(m_stopping);
    if(GetThis() == this){
        SEVER_CC_LOG_INFO(g_logger)<< "~Scheduler";
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis(){
    return t_scheduler;
}

void Scheduler::setThis(){
    t_scheduler = this;
}

Fiber* Scheduler::GetMainFiber(){

    return t_scheduler_fiber;
}

void Scheduler::tickle(){
    SEVER_CC_LOG_DEBUG(g_logger) << "tickle";
}

bool Scheduler::stopping(){
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;
}


void Scheduler::start(){
    MutexType::Lock lock(m_mutex);
    if(!m_stopping){
        return;
    }
    m_stopping = false;
    SEVER_ASSERT(m_threads.empty());

    //启动m_threadNum个线程
    m_threads.resize(m_threadNum);
    for(size_t i = 0; i < m_threadNum; ++i){
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getPid());
    }
    lock.unlock();
}   

void Scheduler::stop(){
    SEVER_CC_LOG_INFO(g_logger) << "start stop";
    m_autoStop = true;
    if(m_rootFiber
            && m_threadNum   == 0
            && (m_rootFiber->getState() == Fiber::TERM
                || m_rootFiber->getState() == Fiber::INIT)) {
        SEVER_CC_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if(stopping()) {
            return;
        }
    }
    //?
    if(m_rootThread != -1) {
        SEVER_ASSERT(GetThis() == this);
    } else {
        SEVER_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    
    for(size_t i = 0; i < m_threadNum; ++i) {
        tickle();
    }

    if(m_rootFiber) {
        tickle();
    }
    if(m_rootFiber) {
       
        if(!stopping()) {
            // SEVER_CC_LOG_INFO(g_logger) << "m_rootFiber call";
          
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }

}


void Scheduler::run(){
    // 设置当前线程的调度器以及主协程
    SEVER_CC_LOG_INFO(g_logger) << m_name << " run";
    setThis();
   
    if(server_cc::GetThreadId() != m_rootThread){
        // SEVER_CC_LOG_INFO(g_logger)<<"run thread not root thread";
        t_scheduler_fiber = Fiber::GetThis().get();
    }
    //  SEVER_CC_LOG_INFO(g_logger)<<"Scheduler: "<<t_scheduler<<" Fiber: "<<t_scheduler_fiber;
    //设置空闲协程，当无任务时执行
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;

    FiberOrCb ft;
    while(true){
        ft.reset();
        bool is_active = false;
        bool tickle_me = false;
        {   
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()){
                if(it->thread != -1 && it->thread != server_cc::GetThreadId()){
                    ++it;
                    tickle_me = true;
                    continue;
                }

                SEVER_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC){
                    ++it;
                    continue;
                }
                ft = *it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            tickle_me |= it != m_fibers.end();
        }
        if(tickle_me){
            tickle();
        }

        if(ft.fiber&&(ft.fiber->getState()!= Fiber::TERM 
                    && ft.fiber->getState()!= Fiber::EXCEPT))
        {
            ft.fiber->swapIn();
            //ft调用完成
            --m_activeThreadCount;

            if(ft.fiber->getState()== Fiber::READY){
                schedule(ft.fiber);
            } 
            else if(ft.fiber->getState() != Fiber::TERM 
                        && ft.fiber->getState() != Fiber::EXCEPT){
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset(); //FiberOrCb结构体reset
        }else if (ft.cb){
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY){
                schedule(cb_fiber);
                cb_fiber.reset();//指针reset
            }else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM){
                
                cb_fiber->reset(nullptr);//fiber的reset函数
            }else{
                SEVER_CC_LOG_INFO(g_logger) << m_name << " run";
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset(); //? 指针reset,m_fiber里面还有这个协程吗
            }
                
        }else {

            if(is_active){
                --m_activeThreadCount;
                continue;
            }
            //? idle_fiber什么时候会 TERM
            if(idle_fiber->getState() == Fiber::TERM){
                SEVER_CC_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            
            if(idle_fiber->getState() != Fiber::TERM
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD;
            }
        }

    }


}

void Scheduler::idle(){
    SEVER_CC_LOG_INFO(g_logger) << "idle";
    while(!stopping()){
        Fiber::YieldToHold();
    }
}
}