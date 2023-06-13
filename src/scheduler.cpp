/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-11 11:35:09
 * @LastEditors: closing
 * @LastEditTime: 2023-06-02 10:51:05
 * @FilePath: /sylar/src/scheduler.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "scheduler.h"
#include "utils.h"
#include "logger.h"
namespace server_cc{

static Logger::ptr g_logger = SERVER_CC_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;// 当前线程的调度器
static thread_local Fiber* t_scheduler_fiber = nullptr;// 当前线程的协程

Scheduler::Scheduler(size_t thread_num, bool use_caller, const std::string& name):m_name(name){
    
    if(use_caller){
        
        Fiber::GetThis();
        --thread_num;
        SERVER_CC_ASSERT(GetThis() == nullptr);
        
        setThis();// 将this 设置当前线程的调度器
        
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
    SERVER_CC_ASSERT(m_stopping);
    if(GetThis() == this){
        SERVER_CC_LOG_INFO(g_logger)<< "~Scheduler";
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
    SERVER_CC_LOG_DEBUG(g_logger) << "tickle";
}

/**
 * @description: 
 * @return {*}  true: 任务队列为空，没有任务在执行
 */
bool Scheduler::stopping(){
    MutexType::Lock lock(m_mutex);
    return  m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;
}


void Scheduler::start(){
    MutexType::Lock lock(m_mutex);
    if(!m_stopping){
        return;
    }
    m_stopping = false;
    SERVER_CC_ASSERT(m_threads.empty());

    //启动m_threadNum个线程
    m_threads.resize(m_threadNum);
    for(size_t i = 0; i < m_threadNum; ++i){
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getPid());
    }
    lock.unlock();
}   

void Scheduler::stop(){
    SERVER_CC_LOG_INFO(g_logger) << "start stop";
    // m_autoStop = true;

    //当user_caller时，如果是当前为主线程，且没有其他线程，且没有协程任务，直接返回
    if(m_rootFiber && m_threadNum   == 0 && 
    (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT)) {
        SERVER_CC_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if(stopping()) {//主要判断是否有协程任务
            return;
        }
    }
    //调用该stop的线程为user_caller的线程，所以如果user_caller==ture则设置了t_scheduler
    //当user_caller为false时，未设置t_scheduler
    if(m_rootThread != -1) {
        SERVER_CC_ASSERT(GetThis() == this);
    } else {
        SERVER_CC_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    
    //唤醒所有处于idle的线程
    for(size_t i = 0; i < m_threadNum; ++i) {
        tickle();
    }
    if(m_rootFiber) {
        tickle();
    }

    //如果user_caller，则在当前线程执行call（调用run函数）
    if(m_rootFiber) {
        
        if(!stopping()) {
            // SERVER_CC_LOG_INFO(g_logger) << "m_rootFiber call";
            
            //程序入口函数，主协程绑定了run函数
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }
    //线程池中每个thread会执行run方法，执行完后线程结束
    for(auto& i : thrs) {
        i->join();
    }

}


void Scheduler::run(){
    // 设置当前线程的调度器以及主协程
    SERVER_CC_LOG_INFO(g_logger) << m_name << " run";
    setThis();
    
    if(server_cc::GetThreadId() != m_rootThread){
        t_scheduler_fiber = Fiber::GetThis().get();
    }
   
    //设置空闲协程，当无任务时执行
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;
    
    FiberOrCb ft;
    while(true){
        ft.reset();
        bool is_active = false;
        bool tickle_me = false;
        //从协程任务队列中取出任务  
        {   
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()){
                if(it->thread != -1 && it->thread != server_cc::GetThreadId()){
                    ++it;
                    tickle_me = true;// 有其他线程的协程任务
                    continue;
                }

                SERVER_CC_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC){
                    ++it;
                    continue;
                }
                
                ft = *it;//按位复制
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            tickle_me |= it != m_fibers.end();
        }
        //当有其他线程的任务协程任务或取出的任务有效时触发tickle()函数
        //（唤醒的协程不是要执行该任务的协程时，会再次调用tickle()函数，
        //直到那个协程被唤醒）
        if(tickle_me){
            tickle();
        }

        //根据任务类型执行，如果是协程任务，执行协程，如果是回调函数，执行回调函数
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
                SERVER_CC_LOG_INFO(g_logger) << m_name << " run";
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset(); //? 指针reset,m_fiber里面还有这个协程吗
            }
                
        }
        //如果没有任务，执行idle协程
        else {

            if(is_active){
                --m_activeThreadCount;
                continue;
            }
            //? idle_fiber什么时候会 TERM：当执行完idle_fiber协程的任务后（跳出while循环），会执行Fiber::run后面的语句，此时idle_fiber的状态为TERM
            if(idle_fiber->getState() == Fiber::TERM){
                SERVER_CC_LOG_INFO(g_logger) << "idle fiber term";
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
    SERVER_CC_LOG_INFO(g_logger) << "idle";
    while(!stopping()){
        Fiber::YieldToHold();
    }
}
}