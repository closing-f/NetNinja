/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-14 18:26:30
 * @LastEditors: closing-f fql2018@bupt.edu.cn
 * @LastEditTime: 2023-05-15 07:54:19
 * @FilePath: /sylar/src/timer.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _TIMER_H
#define _TIMER_H
#include "thread.h"
#include<memory>
#include<set>
#include<vector>
namespace server_cc
{
class TimerManager;
class Timer : public std::enable_shared_from_this<Timer>
{
    friend class TimerManager;
    public:
        typedef std::shared_ptr<Timer> ptr;
        Timer(uint64_t ms,std::function<void()> cb,bool recurring = false,TimerManager* timerManager = NULL);
        Timer(uint64_t next);
        ~Timer();

        bool cancel();//取消定时器
        bool refresh();//刷新定时器
        bool reset(uint64_t ms,bool from_now);//重置定时器时间
    
    private:
        bool m_recurring = false; //是否循环
        uint64_t m_ms = 0;//执行周期
        uint64_t m_next = 0;//下次执行时间

        std::function<void()> m_cb;

        TimerManager* m_manager = nullptr;
    private:
        struct Comparator{
            bool operator()(const Timer::ptr& lhs,const Timer::ptr& rhs) const;
        };
        
};

class TimerManager{
    friend class Timer;
    public:
        typedef RWMutex RWMutexType;
        
        TimerManager();
        virtual ~TimerManager();

        Timer::ptr addTimer(uint64_t ms,std::function<void()> cb,bool recurring = false);

        Timer::ptr addConditionTimer(uint64_t ms,std::function<void()> cb,
                                    std::weak_ptr<void> weak_cond,//如果智能指针引用计数为0时执行
                                    bool recurring = false);

        uint64_t getTimerNext();//获取下一个定时器执行的时间

        void listExpiredCb(std::vector<std::function<void()>>& cbs);//获取当前时间需要执行的定时器回调函数

        void addTimer(Timer::ptr val,RWMutexType::WriteLock& lock);//添加定时器

        bool detectClockRollover(uint64_t now_ms);//检测时钟是否回拨
        bool hasTimer();//是否有定时器
    private:
        RWMutexType m_mutex;
        std::set<Timer::ptr,Timer::Comparator> m_timers;
        bool m_tickled = false;//是否触发定时器
        uint64_t m_previousTime = 0;//上次执行时间

    protected:
        virtual void onTimerInsertedAtFront() = 0;//当有新的定时器插入到定时器集合的最前面时执行
};

}
#endif // !_