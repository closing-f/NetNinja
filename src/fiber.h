/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-05-09 09:48:20
 * @LastEditors: closing-f fql2018@bupt.edu.cn
 * @LastEditTime: 2023-05-16 11:00:05
 * @FilePath: /sylar/src/fiber.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/**
*/
#ifndef _FIBER_H
#define _FIBER_H

#include<memory>
#include<ucontext.h>
#include<functional>
namespace server_cc{
    //? Fiber什么时候析构，
class Fiber:public std::enable_shared_from_this<Fiber>{
    public:
        typedef std::shared_ptr<Fiber> ptr;

    /**
     * @brief 协程状态
     */
    enum State {
        /// 初始化状态
        INIT,
        /// 暂停状态
        HOLD,
        /// 执行中状态
        EXEC,
        /// 结束状态
        TERM,
        /// 可执行状态
        READY,
        /// 异常状态
        EXCEPT
    };

    private:
        /**
         * @brief 无参构造函数
         * @attention 线程中第一个协程（主协程）的构造
        */
        Fiber();
    public:
        Fiber(std::function<void()>cb,size_t stack_size=0,bool use_caller = false);

        ~Fiber();

        void reset(std::function<void()>cb);

        void swapIn();

        void swapOut();

        /**
     * @brief 将当前线程切换到执行状态
     * @pre 执行的为当前线程的主协程
     */
    void call();

    /**
     * @brief 将当前线程切换到后台
     * @pre 执行的为该协程
     * @post 返回到线程的主协程
     */
    void back();

        /**
         * @brief 返回协程id
         */
        uint64_t getId() const { return m_id;}

        /**
         * @brief 返回协程状态
         */
        State getState() const { return m_state;}
    public:
       
        /**
         * @description: 
         * @param {Fiber*} fiber 
         * @return {*}
         */
        static void SetThis(Fiber* fiber);

        /**
         * @description: 
         * @param {Fiber*} fiber
         * @return {*}
         */
        static Fiber::ptr GetThis();

            /**
         * @brief 将当前协程切换到后台,并设置为READY状态
         * @post getState() = READY
         */
        static void YieldToReady();

        /**
         * @brief 将当前协程切换到后台,并设置为HOLD状态
         * @post getState() = HOLD
         */
        static void YieldToHold();

        /**
         * @brief 返回当前协程的总数量
         */
        static uint64_t TotalFibers();

        /**
         * @brief 协程执行函数
         * @post 执行完成返回到线程主协程
         */
        static void MainFunc();


        /**
         * @brief 协程执行函数
         * @post 执行完成返回到线程主协程
         */
        static void CallerMainFunc();
        /**
         * @brief 获取当前协程的id
         */
        static uint64_t GetFiberId();

        State m_state = INIT;
    private:
        // 协程id
        uint64_t m_id = 0;
        // 协程运行栈大小
        uint32_t m_stacksize = 0;
        // 协程状态
        
        // 协程上下文
        ucontext_t m_ctx;
        // 协程运行栈指针
        void* m_stack = nullptr;
        // 协程运行函数
        std::function<void()> m_cb;

};
}

#endif // !_FIBER_H
