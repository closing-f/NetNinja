#include "hook.h"
#include <dlfcn.h>

#include "config.h"
#include "logger.h"
#include "fiber.h"
#include "iomanager.h"
#include "fd_manager.h"
#include "utils.h"

server_cc::Logger::ptr g_logger = SERVER_CC_LOG_NAME("system");

namespace server_cc{

static server_cc::ConfigVar<int>::ptr g_tcp_connect_timeout =
    server_cc::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");
static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);//将C库函数调用修改为name_f的形式，这样就可以在hook中调用系统函数
    HOOK_FUN(XX);
#undef XX
}

static uint64_t s_connect_timeout = -1;
struct _HookIniter {
    _HookIniter() {
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getValue();

        g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
                SERVER_CC_LOG_ERROR(g_logger) << "tcp connect timeout changed from "
                                         << old_value << " to " << new_value;
                s_connect_timeout = new_value;
        });
    }
};

static _HookIniter s_hook_initer;

bool is_hook_enable() {
    return t_hook_enable;
}

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}

}

struct timer_info {
    int cancelled = 0;
};

template<typename OriginFun, typename... Args>//OriginFun是函数指针类型，Args是可变参数模板
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
        uint32_t event, int timeout_so, Args&&... args) {
    
    if(!server_cc::t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }

    server_cc::FdCtx::ptr ctx = server_cc::FdMgr::GetInstance().get(fd);
    if(!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    if(ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket() || ctx->getUserNonblock()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    
    while(n == -1 && errno == EINTR) {//被系统调用中断，重试
        n = fun(fd, std::forward<Args>(args)...);
    }

    if(n == -1 && errno == EAGAIN) {
        server_cc::IOManager* iom = server_cc::IOManager::GetThis();
        server_cc::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        //如果设定了超时时间，就添加定时器，到点后取消事件
        if(to != (uint64_t)-1) {
            timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (server_cc::IOManager::Event)(event));
            }, winfo);
        }
        //在IOManager中添加该事件然后让出协程，等到该事件被触发后，再继续执行
        int rt = iom->addEvent(fd, (server_cc::IOManager::Event)(event));
        if((rt)) {//error
            SERVER_CC_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                << fd << ", " << event << ")";
            if(timer) {
                timer->cancel();
            }
            return -1;
        } else {
            server_cc::Fiber::YieldToHold();
            if(timer) {
                timer->cancel();
            }
            //如果超出了超时时间，就返回错误，不再retry
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }
    }
    
    return n;
}

//兼容C语言的函数调用，需要使用extern "C"修饰。可以实现C 引用 C++ 函数
//? 那么有没有一种可能C++文件引用该C函数呢？名称还能识别出来吗
extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if(!server_cc::t_hook_enable) {
        return sleep_f(seconds);
    }

    server_cc::Fiber::ptr fiber = server_cc::Fiber::GetThis();
    server_cc::IOManager* iom = server_cc::IOManager::GetThis();
    iom->addTimer(seconds * 1000, std::bind((void(server_cc::Scheduler::*)
            (server_cc::Fiber::ptr, int thread))&server_cc::IOManager::schedule
            ,iom, fiber, -1));
    server_cc::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec) {
    if(!server_cc::t_hook_enable) {
        return usleep_f(usec);
    }
    server_cc::Fiber::ptr fiber = server_cc::Fiber::GetThis();
    server_cc::IOManager* iom = server_cc::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(server_cc::Scheduler::*)
            (server_cc::Fiber::ptr, int thread))&server_cc::IOManager::schedule
            ,iom, fiber, -1));
    server_cc::Fiber::YieldToHold();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!server_cc::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 /1000;
    server_cc::Fiber::ptr fiber = server_cc::Fiber::GetThis();
    server_cc::IOManager* iom = server_cc::IOManager::GetThis();
    iom->addTimer(timeout_ms, std::bind((void(server_cc::Scheduler::*)
            (server_cc::Fiber::ptr, int thread))&server_cc::IOManager::schedule
            ,iom, fiber, -1));
    server_cc::Fiber::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol) {
    if(!server_cc::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if(fd == -1) {
        return fd;
    }
    server_cc::FdMgr::GetInstance().get(fd, true);
    return fd;
}

int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms) {
    SERVER_CC_LOG_DEBUG(g_logger)<<"connect_with_timeout";
    if(!server_cc::t_hook_enable) {
        return connect_f(fd, addr, addrlen);
    }
    server_cc::FdCtx::ptr ctx = server_cc::FdMgr::GetInstance().get(fd);
    if(!ctx || ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket()) {
        return connect_f(fd, addr, addrlen);
    }

    if(ctx->getUserNonblock()) {
        return connect_f(fd, addr, addrlen);
    }

    int n = connect_f(fd, addr, addrlen);
    // SERVER_CC_LOG_DEBUG(g_logger)<<"connect_with_timeout n = "<<n;
    if(n == 0) {
        return 0;
    } else if(n != -1 || errno != EINPROGRESS) {
        return n;
    }

    server_cc::IOManager* iom = server_cc::IOManager::GetThis();
    server_cc::Timer::ptr timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);
    SERVER_CC_LOG_DEBUG(g_logger)<<"Connect timeout_ms = "<<timeout_ms;
    if(timeout_ms != (uint64_t)-1) {
        timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, server_cc::IOManager::WRITE);
        }, winfo);
    }
    // SERVER_CC_LOG_DEBUG(g_logger)<< "addEvent(" << fd<< ", WRITE)";
    int rt = iom->addEvent(fd, server_cc::IOManager::WRITE);
    if(rt == 0) {
       
        server_cc::Fiber::YieldToHold();
        if(timer) {
            timer->cancel();
        }
        if(tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    } else {
        if(timer) {
            timer->cancel();
        }
        SERVER_CC_LOG_ERROR(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    if(!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, server_cc::s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = do_io(s, accept_f, "accept", server_cc::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) {
        server_cc::FdMgr::GetInstance().get(fd, true);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", server_cc::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", server_cc::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", server_cc::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", server_cc::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", server_cc::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", server_cc::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", server_cc::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", server_cc::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", server_cc::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", server_cc::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd) {
    if(!server_cc::t_hook_enable) {
        return close_f(fd);
    }

    server_cc::FdCtx::ptr ctx = server_cc::FdMgr::GetInstance().get(fd);
    if(ctx) {
        auto iom = server_cc::IOManager::GetThis();
        if(iom) {
            iom->cancelAll(fd);
        }
        server_cc::FdMgr::GetInstance().del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ ) {
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                server_cc::FdCtx::ptr ctx = server_cc::FdMgr::GetInstance().get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                server_cc::FdCtx::ptr ctx = server_cc::FdMgr::GetInstance().get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if(FIONBIO == request) {
        bool user_nonblock = !!*(int*)arg;
        server_cc::FdCtx::ptr ctx = server_cc::FdMgr::GetInstance().get(d);
        if(!ctx || ctx->isClose() || !ctx->isSocket()) {
            return ioctl_f(d, request, arg);
        }
        ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if(!server_cc::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            server_cc::FdCtx::ptr ctx = server_cc::FdMgr::GetInstance().get(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}
}

