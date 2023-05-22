
#include "fiber.h"
#include "thread.h"
#include "mutex.h"
namespace server_cc
{

class Scheduler
{
    public:
        typedef std::shared_ptr<Scheduler> ptr;
        typedef Mutex MutexType;
        
        /**
         * @description: 构造函数 
         * @param {size_t} thread_num 线程数量
         * @param {bool} use_caller 是否在当前线程中创建协程
         * @param {string&} name    
         * @return {*}
         */        
        Scheduler(size_t thread_num = 1, bool use_caller = true, const std::string& name = "");
        
        virtual ~Scheduler();
        
        const std::string& getName() const { return m_name;}
        template<typename T>
        void schedule(T fc,int thread= -1){
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                need_tickle=scheduleNoLock(fc,thread);   
            }
            if(need_tickle) {
                tickle();
            }
            
        }

        template<class InputIterator>
        void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end) {
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                ++begin;
            }
        }
        if(need_tickle) {
            tickle();
        }
    }
        /**
         * @description: 启动调度器
         * @return {*}
         */        
        void start();
        /**
         * @description: 停止调度器
         * @return {*}
         */        
        void stop();

        static Scheduler* GetThis();
        
        static Fiber* GetMainFiber();


    protected:
            
        void setThis();
    
        virtual void run();
    
        virtual bool stopping();

        virtual void tickle();

        bool hasIdleThreads() { return m_idleThreadCount > 0;}
        /**
     * @brief 协程无任务可调度时执行idle协程
     */
        virtual void idle();
        struct FiberOrCb
        {
            Fiber::ptr fiber;
            std::function<void()> cb;
            int thread = -1;

            /**
         * @brief 构造函数
         * @param[in] f 协程
         * @param[in] thr 线程id
         */
        FiberOrCb(Fiber::ptr f, int thr)
            :fiber(f), thread(thr) {
        }

        /**
         * @brief 构造函数
         * @param[in] f 协程指针
         * @param[in] thr 线程id
         * @post *f = nullptr
         */
        FiberOrCb(Fiber::ptr* f, int thr)
            :thread(thr) {
            fiber.swap(*f);
        }

        /**
         * @brief 构造函数
         * @param[in] f 协程执行函数
         * @param[in] thr 线程id
         */
        FiberOrCb(std::function<void()> f, int thr)
            :cb(f), thread(thr) {
        }

        /**
         * @brief 构造函数
         * @param[in] f 协程执行函数指针
         * @param[in] thr 线程id
         * @post *f = nullptr
         */
        FiberOrCb(std::function<void()>* f, int thr)
            :thread(thr) {
            cb.swap(*f);
        }

        /**
         * @brief 无参构造函数
         */
        FiberOrCb()
            :thread(-1) {
        }


            void reset()
            {
                fiber = nullptr;
                cb = nullptr;
                thread= -1;
            }
        };

    private:
        template<class T>
        bool scheduleNoLock(T fc, int thread) {
            bool need_tickle = m_fibers.empty();
            FiberOrCb ft(fc, thread);
            if(ft.fiber || ft.cb) {
                m_fibers.push_back(ft);
            }
            return need_tickle;
        }
        
        std::vector<Thread::ptr> m_threads;
        
        std::list<FiberOrCb> m_fibers;
        
        Fiber::ptr m_rootFiber;
        
        std::string m_name;
        
        MutexType m_mutex;

        

    protected:

        std::vector<int> m_threadIds;

        // size_t m_threadCount = 0;
        /// 线程数量
        size_t m_threadNum = 0;

        std::atomic<size_t> m_activeThreadCount = {0};

        std::atomic<size_t> m_idleThreadCount = {0};

        
        bool m_stopping = true;//是否正在停止

        bool m_autoStop = false;

        int m_rootThread = 0;
        



};
    
}