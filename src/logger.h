/*
 * @Author: closing
 * @Date: 2023-04-09 00:57:34
 * @LastEditors: closing
 * @LastEditTime: 2023-05-22 11:09:47
 * @Description: logger头文件，定义logger的相关类
 */

#ifndef LOGGER_H
#define LOGGER_H
#include<memory>
#include<string>
#include<list>
#include<fstream>
#include<sstream>
#include<ctime>
#include<vector>
#include<map>
#include<iostream>
#include<functional>
#include"utils.h"
#include "mutex.h"
#include <stdarg.h>
#include "thread.h"
#define SERVER_CC_LOG_LEVEL(logger,level) \
    if(logger->getLevel()<=level) \
        server_cc::LogEventWrapper(server_cc::LogEvent::ptr(new server_cc::LogEvent(logger,level,__FILE__, __LINE__ ,0,server_cc::GetThreadId(),server_cc::GetFiberId(),time(0),server_cc::Thread::GetName()))).getSS()

#define SERVER_CC_LOG_DEBUG(logger) SERVER_CC_LOG_LEVEL(logger,server_cc::LogLevel::DEBUG)
#define SERVER_CC_LOG_WARN(logger) SERVER_CC_LOG_LEVEL(logger,server_cc::LogLevel::WARNING)
#define SERVER_CC_LOG_INFO(logger) SERVER_CC_LOG_LEVEL(logger,server_cc::LogLevel::INFO)
#define SERVER_CC_LOG_ERROR(logger) SERVER_CC_LOG_LEVEL(logger,server_cc::LogLevel::ERROR)
#define SERVER_CC_LOG_FATAL(logger) SERVER_CC_LOG_LEVEL(logger,server_cc::LogLevel::FATAL)


#define SERVER_CC_LOG_FMT_LEVEL(logger,level,fmt,...) \
    if(logger->getLevel()<=level) \
        server_cc::LogEventWrapper(server_cc::LogEvent::ptr(new server_cc::LogEvent(logger,level,__FILE__,server_cc::GetThreadId(),server_cc::GetFiberId(),time(0)))).getEvent()->format(fmt,__VA_ARGS__)

#define SERVER_CC_LOG_FMT_DEBUG(logger,fmt,...) SERVER_CC_LOG_FMT_LEVEL(logger,server_cc::LogLevel::DEBUG,fmt,__VA_ARGS__)
#define SERVER_CC_LOG_FMT_INFO(logger,fmt,...) SERVER_CC_LOG_FMT_LEVEL(logger,server_cc::LogLevel::INFO,fmt,__VA_ARGS__)
#define SERVER_CC_LOG_FMT_WARN(logger,fmt,...) SERVER_CC_LOG_FMT_LEVEL(logger,server_cc::LogLevel::WARNING,fmt,__VA_ARGS__)
#define SERVER_CC_LOG_FMT_ERROR(logger,fmt,...) SERVER_CC_LOG_FMT_LEVEL(logger,server_cc::LogLevel::ERROR,fmt,__VA_ARGS__)
#define SERVER_CC_LOG_FMT_FATAL(logger,fmt,...) SERVER_CC_LOG_FMT_LEVEL(logger,server_cc::LogLevel::FATAL,fmt,__VA_ARGS__)



#define SERVER_CC_LOG_ROOT() server_cc::LoggerMgr::GetInstance().getRoot()

#define SERVER_CC_LOG_NAME(name) server_cc::LoggerMgr::GetInstance().getLogger(name)
namespace server_cc{


class LogLevel {
public:
    
    enum Level {
        UNKNOWN = 0,
        DEBUG=1,
        INFO=2,
        WARNING=3,
        ERROR=4,
        FATAL=5
    };
    
    static const char* getLevel(Level level);
    static Level FromString(const std::string& str){
        if(str=="DEBUG") return Level::DEBUG;
        if(str=="INFO") return Level::INFO;
        if(str=="WARNING") return Level::WARNING;
        if(str=="ERROR") return Level::ERROR;
        if(str=="FATAL") return Level::FATAL;
        return Level::FATAL;
    }
    
    static std::string ToString(Level level){
        switch(level){
            case Level::DEBUG:
                return "DEBUG";
            case Level::INFO:
                return "INFO";
            case Level::WARNING:
                return "WARNING";
            case Level::ERROR:
                return "ERROR";
            case Level::FATAL:
                return "FATAL";
            default:
                return "UNKNOWN";
        }
    }

};

class Logger;
class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;

        /**
         * @description: 构造函数
         * @param {std::shared_ptr<Logger>} logger 日志器
         * @param {LogLevel::Level} level 日志级别
         * @param {const char*} file 文件名
         * @param {int32_t} line 行号
         * @param {uint32_t} elapse 程序启动到现在的毫秒数
         * @param {uint32_t} thread_id  线程id
         * @param {uint32_t} fiber_id   协程id
         * @param {uint64_t} time   时间戳
         * @param {const std::string&} thread_name  线程名称
         * @return {*}
         */ 
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
            ,const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
            ,const std::string& thread_name);
        
        const char* getFile() const { return m_filename; }
        int getLine() const { return m_line; }
        u_int32_t getElapse() const { return m_elapse; }
        u_int32_t getThreadId() const { return m_threadId; }
        u_int32_t getFiberId() const { return m_fiberId; }
        u_int64_t getTime() const { return m_time; }
        std::string getContent() const { return m_ss.str(); }
        std::shared_ptr<Logger> getLogger() const { return m_logger; }
        LogLevel::Level getLevel() const { return m_level; }
        const std::string& getThreadName() const { return m_threadName; }
        /**
         * @description: 
         * @param {char*} fmt 格式
         * @param {va_list} al 可变参数列表
         * @return {*}
         */ 
        void format(const char* fmt, ...);
        void format(const char* fmt, va_list al);
        
        /**
         * @description: 获取日志内容字符串流
         * @return {*}
         */        
        std::stringstream& getSS() { return m_ss; }

        

          
    private:
        const char* m_filename = nullptr;//文件名
        int m_line=0;//行号
        u_int32_t m_elapse=0; //程序启动到现在的毫秒数
        u_int32_t m_threadId=0;//线程id
        u_int32_t m_fiberId=0;//协程id
        u_int64_t m_time=0;//时间戳
        std::string m_content;//日志内容
        std::shared_ptr<Logger> m_logger=nullptr;
        LogLevel::Level m_level;
        const std::string m_threadName;
        std::stringstream m_ss;


};


/**
 * @description: 在析构函数中，将日志内容写入到日志文件中. //?为什么要用wrapper来实现流式风格日志，不直接用LogEvent？
 * @return {*}
 */
class LogEventWrapper{
    public:
        
        LogEventWrapper(LogEvent::ptr event);
        ~LogEventWrapper();
        std::stringstream& getSS();
        LogEvent::ptr getEvent() const { return m_event; }
    private:
        LogEvent::ptr m_event;
};


// 日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    
    std::string format(LogLevel::Level level,LogEvent::ptr event);
    std::ostream& format(std::ostream& os,LogLevel::Level level, LogEvent::ptr event);
    LogFormatter(const std::string& pattern);
    std::string getPattern() const { return m_pattern; }

    void resetPattern(const std::string& pattern);
    
public:
    std::string m_pattern;//日志格式
    class FormatItem {
    public:
        
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, LogEvent::ptr event) = 0;
    };
    void init();
private:
    
    std::string m_error;//错误信息
    std::vector<FormatItem::ptr> m_items;
};


class LogAppender{
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        typedef Mutex MutexType;

        virtual ~LogAppender();
        
        virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;
        void setFormatter(const std::string& formatter);
        LogFormatter::ptr getFormatter() const { return m_formatter; }

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val; }
        virtual std::string ToYamlString() = 0;

    protected:
        LogLevel::Level m_level= LogLevel::DEBUG;
        
        
    public:
        LogFormatter::ptr m_formatter;
        bool m_hasFormatter = false;
        MutexType m_mutex;
};
class FileLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string& filename);
        void log(LogLevel::Level level, LogEvent::ptr event) override;
        // bool reopen();
        std::string ToYamlString() override;
        ~FileLogAppender();
        bool hasFormatter() const { return m_hasFormatter; }
      
    private:
        std::string m_filename;
        std::ofstream m_filestream;
    public:
        bool m_hasFormatter = false;

};

/**
 * @description: 输出到控制台的Appender
 * @return {*}
 */
class StdoutLogAppender : public LogAppender {
    public:
        
        
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(LogLevel::Level level, LogEvent::ptr event) override;
        std::string ToYamlString() override;
        bool hasFormatter() const { return m_hasFormatter; }
        ~StdoutLogAppender(){};
};

/**
 * @description: 日志器
 * @return {*}
 */
class Logger{
    public:
        typedef std::shared_ptr<Logger> ptr;
        typedef Mutex MutexType;
        Logger(const std::string& name="root") : m_name(name), m_level(LogLevel::DEBUG){
            m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
        };
        void log(LogLevel::Level level, LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);
        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        void clearAppenders();
        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val; }
        const std::string getName() const { return m_name; }
        void setName(const std::string& val) { m_name = val; }
        void setFormatter(const std::string&pattern);
        LogFormatter::ptr getFormatter() const { return m_formatter; }
        std::string ToYamlString();
        
    private:
        std::string m_name;//logger名称
        // std::string m_name;//logger名称
        LogLevel::Level m_level;//日志级别
        std::list<LogAppender::ptr> m_appenders;//Appender集合
        LogFormatter::ptr m_formatter;
        MutexType m_mutex;
};

/**
 * @description: 日志器管理类
 * @return {*}
 */
class LoggerManager{
    public:
        typedef Mutex MutexType;
        LoggerManager(const std::string& name="root");
        Logger::ptr getLogger(const std::string& name);
        Logger::ptr getRoot(){return m_root;}
        // std::string toYamlString();
        std::string ToYamlString();
        
    private:
        std::map<std::string, Logger::ptr> m_loggers;
        Logger::ptr m_root;
    public:
        MutexType m_mutex;

};

typedef Singleton<LoggerManager> LoggerMgr;

}
#endif // !