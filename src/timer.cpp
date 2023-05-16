/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-14 18:26:30
 * @LastEditors: closing-f fql2018@bupt.edu.cn
 * @LastEditTime: 2023-05-15 08:35:02
 * @FilePath: /sylar/src/timer.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include "timer.h"
#include "utils.h"  
namespace server_cc{

bool Timer::Comparator::operator()(const Timer::ptr& lhs,const Timer::ptr& rhs) const{
    if(!lhs && !rhs){
        return false;
    }
    if(!lhs){
        return true;
    }
    if(!rhs){
        return false;
    }
    if(lhs->m_next < rhs->m_next){
        return true;
    }
    if(rhs->m_next < lhs->m_next){
        return false;
    }
    //! 一开始未写下面这句话,导致定时器无法正常执行（段错误），因为set容器的特性，当两个元素的key相同时，set会认为这两个元素相同，不会插入
    return lhs.get() < rhs.get();
}

Timer::Timer(uint64_t ms,std::function<void()> cb,bool recurring,TimerManager* timerManager)
    :m_ms(ms),
    m_cb(cb),
    m_recurring(recurring),
    m_manager(timerManager){
    m_next = server_cc::GetCurrentMS() + m_ms;
}

Timer::Timer(uint64_t next)
    :m_next(next){
}
Timer::~Timer(){
    //std::cout << "~Timer" << std::endl;
    //cancel();
}

bool Timer::cancel(){
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(m_cb){
        m_cb = nullptr;
        auto it = m_manager->m_timers.find(shared_from_this());//? shared_from_this与this的区        m_manager->m_timers.erase(it);
        m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}

bool Timer::refresh(){
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb){
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()){
        return false;
    }
    m_manager->m_timers.erase(it);
    m_next = server_cc::GetCurrentMS() + m_ms;
    m_manager->m_timers.insert(shared_from_this());
    return true;
}

bool Timer::reset(uint64_t ms,bool from_now){    
    if(ms == m_ms && !from_now){//立马执行
        return true;
    }
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb){
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()){
        return false;
    }
    m_manager->m_timers.erase(it);
    uint64_t start = 0;
    if(from_now){
        start = server_cc::GetCurrentMS();
    }else{
        start = m_next - m_ms;
    }
    m_ms = ms;
    m_next = start + m_ms;
    m_manager->addTimer(shared_from_this(),lock);
    return true;
}

TimerManager::TimerManager(){
    m_previousTime = server_cc::GetCurrentMS();
}
TimerManager::~TimerManager() {
}
Timer::ptr TimerManager::addTimer(uint64_t ms,std::function<void()> cb,bool recurring){
    Timer::ptr timer(new Timer(ms,cb,recurring,this));
    RWMutexType::WriteLock lock(m_mutex);
    addTimer(timer,lock);
    return timer;
}

void TimerManager::addTimer(Timer::ptr val,RWMutexType::WriteLock& lock){
    auto it = m_timers.insert(val).first;
    bool at_front = (it == m_timers.begin()) && !m_tickled;
    if(at_front){
        m_tickled = true;
    }
    lock.unlock();
    if(at_front){
        onTimerInsertedAtFront();
    }
}

static void OnTimer(std::weak_ptr<void> weak_cond,std::function<void()> cb){
    std::shared_ptr<void> tmp = weak_cond.lock();//返回一个shared_ptr对象,指向weak_ptr指向的对象
    if(tmp){
        cb();
    }
}


Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring){
    
    return addTimer(ms,std::bind(&OnTimer,weak_cond,cb),recurring);

}

uint64_t TimerManager::getTimerNext(){
    RWMutexType::ReadLock lock(m_mutex);
    m_tickled = false;
    if(m_timers.empty()){
        return ~0ull;//返回最大的无符号整数
    }
    const Timer::ptr& next = *(m_timers.begin());
    uint64_t now_ms = server_cc::GetCurrentMS();
    if(now_ms >= next->m_next){
        return 0;
    }else{
        return next->m_next - now_ms;
    }   
}

void TimerManager::listExpiredCb( std::vector<std::function<void()>>& cbs){
    uint64_t now_ms = server_cc::GetCurrentMS();
    std::vector<Timer::ptr> expired;
    {   
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timers.empty()){
            return;
        }
    }
    bool rollover = detectClockRollover(now_ms);
   
    if (!rollover && ((*(m_timers.begin()))->m_next > now_ms)){
        return;
    }

    
    RWMutexType::WriteLock lock(m_mutex);
    Timer::ptr now_timer(new Timer(now_ms));
    //如果回拨了时间，那么把所有的定时器都当成过期的定时器
    auto it = rollover? m_timers.end() : m_timers.lower_bound(now_timer);//? lower_bound函数的作用
    while(it != m_timers.end() && (*it)->m_next == now_ms){
        ++it;
    }
    expired.insert(expired.begin(),m_timers.begin(),it);
    m_timers.erase(m_timers.begin(),it);//清除过期的定时器
    
    cbs.reserve(expired.size());//? reserve函数的作用
    for(auto& timer : expired){
        cbs.push_back(timer->m_cb);
        if(timer->m_recurring){
            timer->m_next = now_ms + timer->m_ms;
            m_timers.insert(timer);
        }else{
            timer->m_cb = nullptr;
        }
    }
}

bool TimerManager::detectClockRollover(uint64_t now_ms){
    bool rollover = false;
    if(now_ms < m_previousTime && now_ms < (m_previousTime - 60 * 60 * 1000)){
        rollover = true;
    }
    m_previousTime = now_ms;
    return rollover;
}

bool TimerManager::hasTimer(){
    RWMutexType::ReadLock lock(m_mutex);
    return !m_timers.empty();
}

}