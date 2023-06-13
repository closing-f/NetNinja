/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-13 07:48:17
 * @LastEditors: closing
 * @LastEditTime: 2023-06-02 11:00:19
 * @FilePath: /sylar/src/iomanager.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _IOMANAGER_H
#define _IOMANAGER_H
#include "scheduler.h"
#include "timer.h"
namespace server_cc{
class IOManager : public Scheduler , public TimerManager{
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;

    enum Event {
        NONE  = 0x0,
        READ  = 0x1,
        WRITE = 0x4,
    };
private:
    struct FdContext{
        typedef Mutex MutexType;
        
        struct EventContext{
            Scheduler* scheduler = nullptr; //事件执行的scheduler
            Fiber::ptr fiber; //事件协程
            std::function<void()> cb; //事件的回调函数
        };
        /**
         * @brief 获取事件上下文类
         * @param[in] event 事件类型
         * @return 返回对应事件的上线文
         */
        EventContext& getContext(Event event);

        /**
         * @brief 重置事件上下文
         * @param[in, out] ctx 待重置的上下文类
         */
        void resetContext(EventContext& ctx);

        /**
         * @brief 触发事件
         * @param[in] event 事件类型
         */
        void triggerEvent(Event event);
        
        EventContext read;//读事件
        EventContext write;//写事件
        int fd;//事件关联的句柄
        Event events = NONE;//已经注册的事件
        MutexType mutex;//该事件的读写锁
    };
public:
    IOManager(size_t thread_num = 1, bool use_caller = true, const std::string& name = "");
    ~IOManager();

    //0 success, -1 error
    /**
     * @description: 添加事件
     * @param {int} fd  事件关联的句柄
     * @param {Event} event 事件类型
     * @param {function<void()>} cb 事件回调函数
     * @return {*}
     */    
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);
    bool cancelAll(int fd);

    static IOManager* GetThis();
protected:

    /**
     * @description: 唤醒idle协程（epoll_wait状态），如果有io事件则处理，否则将控制权返回到主协程
     * @return {*}
     */    
    void tickle() override;
    bool stopping() override;
    void idle() override;
    bool stopping(uint64_t& timeout);
    void onTimerInsertedAtFront() override;
    /**
     * @brief 重置socket句柄上下文的容器大小
     * @param[in] size 容量大小
     */
    void contextResize(size_t size);
private:
    int m_epfd = 0;//epoll句柄
    int m_tickleFds[2];//用于唤醒epoll_wait的管道
    std::atomic<size_t> m_pendingEventCount = {0};//当前待处理事件数量
    
    RWMutexType m_mutex;
    std::vector<FdContext*> m_fdContexts;//socket句柄上下文的容器
};
}










#endif // ! 