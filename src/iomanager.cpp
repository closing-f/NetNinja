/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-13 07:48:12
 * @LastEditors: closing
 * @LastEditTime: 2023-06-02 10:07:45
 * @FilePath: /sylar/src/iomanager.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils.h"
#include "iomanager.h"
#include "logger.h"
#include <errno.h>
#include<iostream>
#include <errno.h>
#include <string.h>

namespace server_cc{
static server_cc::Logger::ptr g_logger = SERVER_CC_LOG_NAME("system");

enum EpollCtlOp {
};
IOManager::FdContext::EventContext& IOManager::FdContext::getContext(IOManager::Event event) {
    switch(event) {
        case IOManager::READ:
            return read;
        case IOManager::WRITE:
            return write;
        default:
            SERVER_CC_ASSERT2(false, "getContext");
    }
    throw std::invalid_argument("getContext invalid event");
}

void IOManager::FdContext::resetContext(EventContext& ctx) {
    ctx.scheduler = nullptr;
    ctx.fiber.reset();
    ctx.cb = nullptr;
}

void IOManager::FdContext::triggerEvent(IOManager::Event event) {
    
    SERVER_CC_ASSERT(events & event);
    
    events = (Event)(events & ~event);//当前事件已经被触发，重置已经注册的事件
    EventContext& ctx = getContext(event);//获取事件上下文，在添加事件时设置
    if(ctx.cb) {
        ctx.scheduler->schedule(&ctx.cb);
    } else {
        ctx.scheduler->schedule(&ctx.fiber);
    }
    ctx.scheduler = nullptr;
    return;
}
IOManager::IOManager(size_t thread_num, bool use_caller, const std::string& name)
    :Scheduler(thread_num, use_caller, name){
    m_epfd = epoll_create(5000);
    SERVER_CC_ASSERT(m_epfd>0);
    
    int rt = pipe(m_tickleFds);//创建管道,用于通知
    
    SERVER_CC_ASSERT(!rt);

    epoll_event event;
    memset(&event,0,sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_tickleFds[0];
    
    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    SERVER_CC_ASSERT(!rt);

    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0],&event);
    SERVER_CC_ASSERT(!rt);

    contextResize(32);
    start();
}
IOManager::~IOManager(){
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    for(size_t i = 0; i<m_fdContexts.size(); ++i){
        if(m_fdContexts[i]){
            delete m_fdContexts[i];
        }
    }
    
}
void IOManager::contextResize(size_t size) {
    m_fdContexts.resize(size);

    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }
}
//0 success, -1 error
int IOManager::addEvent(int fd, Event new_event, std::function<void()> cb){
    
    //扩展fd上下文数组(fdContexts)的容量
    FdContext* fd_ctx = nullptr;
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() > fd) {
        fd_ctx = m_fdContexts[fd];
        lock.unlock();
    } else {
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        contextResize(fd * 1.5);
        fd_ctx = m_fdContexts[fd];
    }

    //判断fd_ctx是否已经注册事件new_event
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if((fd_ctx->events & new_event)) {
        SERVER_CC_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                    << " new_event=" << (EPOLL_EVENTS)new_event
                    << " fd_ctx.event=" << (EPOLL_EVENTS)fd_ctx->events;
        SERVER_CC_ASSERT(!(fd_ctx->events & new_event));
    }

    //判断是修改事件还是添加事件，将fd_ctx作为data.ptr传入
    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->events | new_event;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        SERVER_CC_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->events;
        return -1;
    }

    //设置事件计数，设置fd_ctx的事件，事件上下文（scheduler，fiber，cb）
    //有回调函数则设置，否则设置协程为当前协程
    ++m_pendingEventCount;
    fd_ctx->events = (Event)(fd_ctx->events | new_event);
    FdContext::EventContext& event_ctx = fd_ctx->getContext(new_event);
    SERVER_CC_ASSERT(!event_ctx.scheduler
                && !event_ctx.fiber
                && !event_ctx.cb);

    event_ctx.scheduler = Scheduler::GetThis();
    if(cb) {
        event_ctx.cb.swap(cb);
    } else {
        event_ctx.fiber = Fiber::GetThis();
        SERVER_CC_ASSERT2(event_ctx.fiber->getState() == Fiber::EXEC
                      ,"state=" << event_ctx.fiber->getState());
    }
    return 0;
}
bool IOManager::delEvent(int fd, Event event){
  RWMutexType::ReadLock lock(m_mutex); 
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if((!(fd_ctx->events & event))) {
        return false;
    }

    Event new_events = (Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        SERVER_CC_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    --m_pendingEventCount;
    fd_ctx->events = new_events;
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}
bool IOManager::cancelEvent(int fd, Event event){
    SERVER_CC_LOG_INFO(g_logger) << "cancelEvent(" << fd << ", " << event << ")";
    RWMutexType::ReadLock lock(m_mutex); 
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if((!(fd_ctx->events & event))) {
        return false;
    }

    Event new_events = (Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        SERVER_CC_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    fd_ctx->triggerEvent(event);
    --m_pendingEventCount;
    return true;
}
bool IOManager::cancelAll(int fd){

    RWMutexType::ReadLock lock(m_mutex); 
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!fd_ctx->events) {
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;
    
    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        SERVER_CC_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    if(fd_ctx->events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    if(fd_ctx->events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
    }

    SERVER_CC_ASSERT(fd_ctx->events == 0);
    return true;
}
IOManager* IOManager::GetThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::tickle() {
    if(!hasIdleThreads()) {
        return;
    }
    
    int rt = write(m_tickleFds[1], "T", 1);
    SERVER_CC_ASSERT(rt == 1);
}
bool IOManager::stopping() {
    uint64_t timeout = 0;
    return stopping(timeout);
}
bool IOManager::stopping(uint64_t& timeout) {
    timeout = getTimerNext();
    return timeout == ~0ull
        && m_pendingEventCount == 0
        && Scheduler::stopping();

}
void IOManager::idle() {
    SERVER_CC_LOG_DEBUG(g_logger)<<"idle";
    const uint64_t MAX_EVNETS = 256;
    
    epoll_event* events = new epoll_event[MAX_EVNETS]();
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
        delete[] ptr;
    });

    while(true) {
        uint64_t next_timeout = 0;
        //TODO stopping
        //判断是否停止并获取下一个定时器的超时时间，当定时器没有任务而且没有要处理的事件时，返回true，idle线程退出
        if((stopping(next_timeout))) {//
            SERVER_CC_LOG_INFO(g_logger) << "name=" << getName()
                                     << " idle stopping exit";
            break;
        }

        //epoll_wait
        int rt = 0;
        do {
            static const int MAX_TIMEOUT = 3000;
            if(next_timeout != ~0ull) {
                next_timeout = (int)next_timeout > MAX_TIMEOUT
                                ? MAX_TIMEOUT : next_timeout;
            } else {
                next_timeout = MAX_TIMEOUT;
            }
            rt = epoll_wait(m_epfd, events, MAX_EVNETS, (int)next_timeout);
            if(rt < 0 && errno == EINTR) {//? errno作用域
            } else {
                break;
            }
        } while(true);;

        //处理超时定时器，执行回调函数；如果为循环定时器，则重新添加到定时器容器
        std::vector<std::function<void()> > cbs;
        listExpiredCb(cbs);
        if(!cbs.empty()) {
            SERVER_CC_LOG_DEBUG(g_logger) << "on timer cbs.size=" << cbs.size();
            schedule(cbs.begin(), cbs.end());
            cbs.clear();
        }

        for(int i = 0; i < rt; ++i) {
            epoll_event& event = events[i];
            if(event.data.fd == m_tickleFds[0]) {
                SERVER_CC_LOG_INFO(g_logger) << "name=" << getName()
                                         << " idle tickle";
                uint8_t dummy[256];
                while(read(m_tickleFds[0], dummy, sizeof(dummy)) > 0);

                continue;
            }

            //获取事件上下文，在添加事件时，将fd_ctx作为data.ptr传入
            FdContext* fd_ctx = (FdContext*)event.data.ptr;
            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            if(event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
            }
            int real_events = NONE;
            if(event.events & EPOLLIN) {
                real_events |= READ;
            }
            if(event.events & EPOLLOUT) {
                real_events |= WRITE;
            }

            if((fd_ctx->events & real_events) == NONE) {
                continue;
            }
            //是否还有剩余未处理事件，有则op为修改事件，否则删除事件
            int left_events = (fd_ctx->events & ~real_events);
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;

            int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if(rt2) {
                SERVER_CC_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                    << (EpollCtlOp)op << ", " << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                    << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                continue;
            }

            //SERVER_CC_LOG_INFO(g_logger) << " fd=" << fd_ctx->fd << " events=" << fd_ctx->events
            //                         << " real_events=" << real_events;
            
            //触发读写事件，设置事件计数，重置事件上下文
            if(real_events & READ) {
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }
            if(real_events & WRITE) {
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }
        }

        Fiber::ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();

        raw_ptr->swapOut();
    }
}

void IOManager::onTimerInsertedAtFront() {
    tickle();
}

}