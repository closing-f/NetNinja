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
#include"../utils.h"
#include <stdarg.h>
#define SEVER_CC_LOG_LEVEL(logger,level) \
    if(logger->getLevel()<=level)\ 
        server_cc::LogEventWrapper(server_cc::LogEvent::ptr(new server_cc::LogEvent(logger,level,__FILE__,server_cc::GetThreadId(),server_cc::GetFiberId(),time(0)))).getSS()

#define SEVER_CC_LOG_DEBUG(logger) SEVER_CC_LOG_LEVEL(logger,server_cc::LogLevel::DEBUG)
#define SEVER_CC_LOG_INFO(logger) SEVER_CC_LOG_LEVEL(logger,server_cc::LogLevel::INFO)
#define SEVER_CC_LOG_WARN(logger) SEVER_CC_LOG_LEVEL(logger,server_cc::LogLevel::WARNING)
#define SEVER_CC_LOG_ERROR(logger) SEVER_CC_LOG_LEVEL(logger,server_cc::LogLevel::ERROR)
#define SEVER_CC_LOG_FATAL(logger) SEVER_CC_LOG_LEVEL(logger,server_cc::LogLevel::FATAL)


#define SEVER_CC_LOG_FMT_LEVEL(logger,level,fmt,...) \
    if(logger->getLevel()<=level)\ 
        server_cc::LogEventWrapper(server_cc::LogEvent::ptr(new server_cc::LogEvent(logger,level,__FILE__,server_cc::GetThreadId(),server_cc::GetFiberId(),time(0)))).getEvent()->format(fmt,__VA_ARGS__)

#define SEVER_CC_LOG_FMT_DEBUG(logger,fmt,...) SEVER_CC_LOG_FMT_LEVEL(logger,server_cc::LogLevel::DEBUG,fmt,__VA_ARGS__)
#define SEVER_CC_LOG_FMT_INFO(logger,fmt,...) SEVER_CC_LOG_FMT_LEVEL(logger,server_cc::LogLevel::INFO,fmt,__VA_ARGS__)
#define SEVER_CC_LOG_FMT_WARN(logger,fmt,...) SEVER_CC_LOG_FMT_LEVEL(logger,server_cc::LogLevel::WARNING,fmt,__VA_ARGS__)
#define SEVER_CC_LOG_FMT_ERROR(logger,fmt,...) SEVER_CC_LOG_FMT_LEVEL(logger,server_cc::LogLevel::ERROR,fmt,__VA_ARGS__)
#define SEVER_CC_LOG_FMT_FATAL(logger,fmt,...) SEVER_CC_LOG_FMT_LEVEL(logger,server_cc::LogLevel::FATAL,fmt,__VA_ARGS__)


#define SEVER_CC_LOG_ROOT() server_cc::LoggerMgr::GetInstance()->getRoot()

namespace server_cc{
class Logger;
class LogLevel {
public:
    enum Level {
        DEBUG=1,
        INFO=2,
        WARNING=3,
        ERROR=4,
        FATAL=5
    };
    static const char* getLevel(Level level);

};
class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time, const std::string& thread_name);//TODO
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, uint32_t thread_id, uint32_t fiber_id, uint64_t time);//TODO
        LogEvent(){};
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
        void format(const char* fmt, ...);
        void format(const char* fmt, va_list al);
        //实现流式风格日志
        std::stringstream& getSS() { return m_ss; }

        

          
    private:
        const char* m_filename = nullptr;//文件名
        int m_line=0;//行号
        u_int32_t m_elapse=0; //程序启动到现在的毫秒数
        u_int32_t m_threadId=0;//线程id
        u_int32_t m_fiberId=0;//协程id
        u_int64_t m_time=0;//时间戳
        std::string m_content;//日志内容
        std::shared_ptr<Logger> m_logger;
        LogLevel::Level m_level;
        const std::string m_threadName;
        std::stringstream m_ss;


};

//?为什么要用wrapper来实现流式风格日志，不直接用LogEvent？
//在析构函数中，将日志内容写入到日志文件中，妙啊
class LogEventWrapper{
    public:
        // LogEventWrapper(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line);
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
    // void resetPattern(const std::string& pattern);



    

public:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, LogEvent::ptr event) = 0;
    };
    void init();
private:
    std::string m_pattern;//日志格式
    std::string m_error;//错误信息
    std::vector<FormatItem::ptr> m_items;
};

//日志输出地
class LogAppender{
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        

        virtual ~LogAppender();
        
        virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;
        void setFormatter(LogFormatter::ptr val);
        LogFormatter::ptr getFormatter() const { return m_formatter; }

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val; }

    protected:
        LogLevel::Level m_level= LogLevel::DEBUG;
        
    public:
        LogFormatter::ptr m_formatter;
        bool m_hasFormatter = false;

};
class FileLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string& filename);
        void log(LogLevel::Level level, LogEvent::ptr event) override;
        // bool reopen();
        ~FileLogAppender();
        bool hasFormatter() const { return m_hasFormatter; }
      
    private:
        std::string m_filename;
        std::ofstream m_filestream;
    public:
        bool m_hasFormatter = false;

};

//标准输出日志器
class StdoutLogAppender : public LogAppender {
    public:
        
        // StdoutLogAppender(LogLevel::Level level = LogLevel::DEBUG) : m_level(level) {}
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(LogLevel::Level level, LogEvent::ptr event) override;
        bool hasFormatter() const { return m_hasFormatter; }
        ~StdoutLogAppender(){};
};

//日志器
class Logger{
    public:
        typedef std::shared_ptr<Logger> ptr;
        Logger(const std::string& name="root") : m_name(name), m_level(LogLevel::DEBUG){
            m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%f%T[%p]%T[%c]:%T%m %n"));
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
        void setFormatter(LogFormatter::ptr val);
        LogFormatter::ptr getFormatter() const { return m_formatter; }
        std::string m_name;//logger名称
    private:
        // std::string m_name;//logger名称
        LogLevel::Level m_level;//日志级别
        std::list<LogAppender::ptr> m_appenders;//Appender集合
        LogFormatter::ptr m_formatter;
};

//文件日志器

class LoggerManager{
    public:
        LoggerManager(const std::string& name="root");
        Logger::ptr getLogger(const std::string& name);
        void init();
        Logger::ptr getRoot(){return m_root;}
        // std::string toYamlString();
        
    private:
        std::map<std::string, Logger::ptr> m_loggers;
        Logger::ptr m_root;

};

typedef Singleton<LoggerManager> LoggerMgr;

}
