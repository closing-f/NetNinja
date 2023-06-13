/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-09 09:48:39
 * @LastEditors: closing
 * @LastEditTime: 2023-06-02 08:49:19
 * @FilePath: /sylar/src/fiber.cpp
 * @Description: 
 */
#include "fiber.h"
#include "utils.h"

#include "logger.h"
#include"config.h"
#include "scheduler.h"
#include <atomic>
namespace server_cc{
    
static Logger::ptr g_logger = SERVER_CC_LOG_NAME("system");

//? =运算符被禁止
static std::atomic<uint64_t> s_fiber_count {0};//协程总数
static std::atomic<uint64_t> s_fiber_id {0};

static thread_local Fiber* t_fiber=nullptr;//当前协程
//? 为何用智能指针而t_fiber用原生指针
static thread_local Fiber::ptr t_threadFiber=nullptr; //主协程

static ConfigVar<uint32_t>::ptr g_fiber_stacksize = Config::Lookup<uint32_t>("fiber.stacksize",128*1024,"fiber_stacksize");

//malloc 当调用者请求的内存超过临界值，malloc 分配的内存是进程的映射区，和动态库的内存占用相同区域；小于临界值时，分配的内存才是在堆上
class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;


Fiber::Fiber(){
    m_state = EXEC;
    SetThis(this);
    
    
    if(getcontext(&m_ctx)){//获取当前上下文
        SERVER_CC_ASSERT2(false,"getcontext");
    }
    ++s_fiber_count;
    SERVER_CC_LOG_INFO(g_logger)<<"Fiber main construction completed";
}

Fiber::Fiber(std::function<void()>cb,size_t stack_size,bool use_caller):m_id(++s_fiber_id),m_cb(cb){
    
    ++s_fiber_count;
    m_stacksize= stack_size? stack_size : g_fiber_stacksize->getValue();
    
    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx)){
        SERVER_CC_ASSERT2(false,"getcontext");
    }
    
    //? uclink设置成null,让其运行完后即终止
    m_ctx.uc_link=nullptr;
    m_ctx.uc_stack.ss_size=m_stacksize;
    m_ctx.uc_stack.ss_sp= m_stack;

    //上下文通过setcontext或者swapcontext激活后，执行func函数，argc为func的参数个数，后面是func的参数序列。
    //当func执行返回后，继承的上下文被激活，如果继承上下文为NULL时，线程退出
    if(!use_caller) {
        //重定向上下文
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }
    SERVER_CC_LOG_INFO(g_logger) << "Fiber::Fiber id=" << m_id;
}

Fiber::~Fiber(){
    
    --s_fiber_count;
    if(m_stack) {
        SERVER_CC_ASSERT(m_state == TERM
                || m_state == EXCEPT
                || m_state == INIT);

        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        //主协程退出
        SERVER_CC_ASSERT(!m_cb);
        SERVER_CC_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    SERVER_CC_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id
                              << " total=" << s_fiber_count;

}

void Fiber::reset(std::function<void()>cb){
    SERVER_CC_ASSERT(m_stack);
    SERVER_CC_ASSERT(m_state == TERM || m_state ==EXCEPT || m_state==INIT);
    m_cb=cb;

    if(getcontext(&m_ctx)){
        SERVER_CC_ASSERT2(false,"getcontext");
    }

    m_ctx.uc_link=nullptr;
    m_ctx.uc_stack.ss_size=m_stacksize;
    m_ctx.uc_stack.ss_sp= m_stack;

    //上下文通过setcontext或者swapcontext激活后，执行func函数，argc为func的参数个数，后面是func的参数序列。
    //当func执行返回后，继承的上下文被激活，如果继承上下文为NULL时，线程退出
    makecontext(&m_ctx,&Fiber::MainFunc,0);
    m_state = INIT;

}

void Fiber::swapIn(){
    SetThis(this);
    SERVER_CC_ASSERT(m_state != EXEC);
    m_state = EXEC;
    //swapcontext 把当前上下文保存到GetMainFiber()->m_ctx中，然后激活m_ctx
    if(swapcontext(&(Scheduler::GetMainFiber()->m_ctx),&m_ctx)){
        SERVER_CC_ASSERT2(false,"swap in context");
    }


}

void Fiber::swapOut(){
    SetThis(Scheduler::GetMainFiber());
    //swapcontext 把当前上下文保存到m_ctx中，然后激活t_threadFiber->m_ctx
    if(swapcontext(&m_ctx,&Scheduler::GetMainFiber()->m_ctx)){
        SERVER_CC_ASSERT2(false,"swap in context");
    }
}

void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        SERVER_CC_ASSERT2(false, "swapcontext");
    }
}

void Fiber::back() {
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        SERVER_CC_ASSERT2(false, "swapcontext");
    }
}
/**
 * @description: 
 * @param {Fiber*} fiber
 * @return {*}
 */
void Fiber::SetThis(Fiber* fiber){
    t_fiber = fiber;
}

/**
 * @description: 
 * @param {Fiber*} fiber
 * @return {*}
 */
Fiber::ptr Fiber::GetThis(){
    if(t_fiber){

        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);
    
    SERVER_CC_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();

}

/**
 * @brief 将当前协程切换到后台,并设置为READY状态
 * @post getState() = READY
 */
void Fiber::YieldToReady(){
    Fiber::ptr cur = GetThis();
    SERVER_CC_ASSERT(cur->m_state == EXEC);
    cur->m_state = READY;
    cur->swapOut();
}

/**
 * @brief 将当前协程切换到后台,并设置为HOLD状态
 * @post getState() = HOLD
 */
void Fiber::YieldToHold(){
    Fiber::ptr cur = GetThis();
    SERVER_CC_ASSERT(cur->m_state == EXEC);
    cur->m_state = HOLD;
    cur->swapOut();
}

/**
 * @brief 返回当前协程的总数量
 */
uint64_t Fiber::TotalFibers(){
    return s_fiber_count;
}

void Fiber::MainFunc(){
    
    Fiber::ptr cur = GetThis();
    SERVER_CC_ASSERT(cur);
    
    try {
        cur->m_cb();
        
        //运行完后置空,state设置为TERM
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        SERVER_CC_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << server_cc::BackTraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        SERVER_CC_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << server_cc::BackTraceToString();
    }

    //? 为什么不直接让cur->swapOut呢？
    // swapcontext触发后不会引用uc_link为null而使得线程终止，因为在该函数中直接swapOut了
    auto raw_ptr = cur.get();
    //cur为shared_ptr从shared_from_this中获取。 reset后引用计数减一。
    cur.reset();
    raw_ptr->swapOut();

    SERVER_CC_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}
        
/**
 * @brief 获取当前协程的id
 */
uint64_t Fiber::GetFiberId(){
    if(t_fiber) {
        // std::cout<<"get fiberId "<<t_fiber->getId()<<std::endl;
        return t_fiber->getId();
    }
    // std::cout<<"not get fiberId "<<std::endl;
    return 0;
}

void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();
    SERVER_CC_ASSERT(cur);
    try {
        cur->m_cb();//执行协程函数,Scheduler::run()
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        SERVER_CC_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << server_cc::BackTraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        SERVER_CC_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << server_cc::BackTraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    SERVER_CC_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));

}

}