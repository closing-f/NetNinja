#include "logger.h"

namespace server_cc{


class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getContent();
    }
};
class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << LogLevel::getLevel(event->getLevel());
    }
};
class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};
class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLogger()->getName();
    }
};
class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};
class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};
class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S") : m_format(format) {
        if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};
class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFile();
    }
};
class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLine();
    }
};
class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << std::endl;
    }
};
class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str) : m_string(str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};
class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << "\t";
    }
};
class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};





//LogEvent初始化 logger level file line elapse thread_id fiber_id time thread_name
LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time, const std::string& thread_name)
    :m_filename(file), m_line(line), m_elapse(elapse), m_threadId(thread_id), m_fiberId(fiber_id), m_time(time), m_threadName(thread_name), m_logger(logger), m_level(level) {
}
//logger level file thread_id fiber_id time
LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file,  uint32_t thread_id, uint32_t fiber_id, uint64_t time)
    :m_filename(file), m_threadId(thread_id), m_fiberId(fiber_id), m_time(time),  m_logger(logger), m_level(level) {
}
void LogEvent::format(const char* fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}
void LogEvent::format(const char* fmt, va_list al) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if(len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

const char* LogLevel::getLevel(LogLevel::Level level) {
    static const char* s_level[] = {
        "UNKNOW",
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "FATAL"
    };
    return s_level[level];
}


LogEventWrapper::LogEventWrapper(LogEvent::ptr e)
    :m_event(e) {
}
LogEventWrapper::~LogEventWrapper() {

    m_event->getLogger()->log(m_event->getLevel(), m_event);
}
std::stringstream& LogEventWrapper::getSS() {
    return m_event->getSS();
}


//调用appenders的log方法
void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        for(auto& i : m_appenders) {
            // std::cout << "log" << std::endl;
            i->log(level, event);
        }
    }
}
void Logger::addAppender(LogAppender::ptr appender) {
    if(!appender->getFormatter()) {
        
        appender->m_formatter = m_formatter;
    }
    m_appenders.push_back(appender);
    
}
void Logger::delAppender(LogAppender::ptr appender) {
    for(auto it = m_appenders.begin();
            it != m_appenders.end(); ++it) {
        if(*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}
void Logger::clearAppenders() {
    m_appenders.clear();
}
void Logger::setFormatter(LogFormatter::ptr val) {
    m_formatter = val;
    for(auto& i : m_appenders) {
        
        if(!i->m_hasFormatter) {
            i->m_formatter = m_formatter;
            i->m_hasFormatter = true;
        }
    }
    
}

void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}
void Logger::info(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}
void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARNING, event);
}
void Logger::error(LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}
void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
}
FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename) {
    m_filestream.open(m_filename);
}
FileLogAppender::~FileLogAppender() {
    // std::cout << "~FileLogAppender()" << std::endl;
    if(m_filestream) {
        m_filestream.close();
    }
}

void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        m_filestream << m_formatter->format(level,event);
    }
}
void StdoutLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        std::cout << m_formatter->format(level,event);
    }

}



//LogFormatter初始化
LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) {
    init();
}
// void LogFormatter::resetPattern(const std::string& pattern) {
//     m_items.clear();
//     m_pattern = pattern;
//     init();
// }
//string格式化
std::string LogFormatter::format(LogLevel::Level level,LogEvent::ptr event) {
    std::stringstream ss;
    for(auto& i : m_items) {
        i->format(ss, event);
    }
    return ss.str();
}
void LogAppender::setFormatter(LogFormatter::ptr val) {
    m_formatter = val;
    m_hasFormatter = true;
}
LogAppender::~LogAppender() {
    // std::cout << "~LogAppender()" << std::endl;
    
}
//ostream格式化
std::ostream& LogFormatter::format(std::ostream& ofs, LogLevel::Level level,LogEvent::ptr event) {
    for(auto& i : m_items) {
        i->format(ofs, event);
    }
    return ofs;
}

//解析pattern
void LogFormatter::init(){
    
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for(size_t i=0;i<m_pattern.size();++i) {
        
        
        if(m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }
        // %%
        if((i+1) < m_pattern.size()) {
            if(m_pattern[i+1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }
        size_t n = i+1;
        int fmt_status = 0;
        size_t fmt_begin = 0;
        std::string str;
        std::string fmt;
        while(n < m_pattern.size()) {

            //类似%d的处理方式，大多此时n为空格
            if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}')) {
                str = m_pattern.substr(i+1, n-i-1);//str为d
                break;
            }
            if(fmt_status == 0) {
                if(m_pattern[n] == '{') {
                    str = m_pattern.substr(i+1, n-i-1);
                    fmt_status = 1;
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if(fmt_status == 1) {
                if(m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin+1, n-fmt_begin-1);//只有{}才有fmt
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            //最后一个字符
            if(n == m_pattern.size()) {
                if(str.empty()) {
                    str = m_pattern.substr(i+1);
                }
            }
        }
        if(fmt_status == 0) {
            //在处理%d的时候之前所有的字符都是nstr
            if(!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, "", 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            
            i = n-1;//for循环最后会++i,所以这里要-1
        } else if(fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }
    //最后一个字符
    if(!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    static std::map<std::string,std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
        {"m", [](const std::string& fmt) { return FormatItem::ptr(new MessageFormatItem(fmt)); }},
        {"p", [](const std::string& fmt) { return FormatItem::ptr(new LevelFormatItem(fmt)); }},
        {"r", [](const std::string& fmt) { return FormatItem::ptr(new ElapseFormatItem(fmt)); }},
        {"c", [](const std::string& fmt) { return FormatItem::ptr(new NameFormatItem(fmt)); }},
        {"t", [](const std::string& fmt) { return FormatItem::ptr(new ThreadIdFormatItem(fmt)); }},
        {"n", [](const std::string& fmt) { return FormatItem::ptr(new NewLineFormatItem(fmt)); }},
        {"d", [](const std::string& fmt) { return FormatItem::ptr(new DateTimeFormatItem(fmt)); }},
        {"f", [](const std::string& fmt) { return FormatItem::ptr(new FiberIdFormatItem(fmt)); }},
        {"T", [](const std::string& fmt) { return FormatItem::ptr(new TabFormatItem(fmt)); }},
    };

    for(auto& i:vec){
        //获得容器i的第三个元素type，即是否为%d
        if(std::get<2>(i) == 0){
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        }else{
            auto it = s_format_items.find(std::get<0>(i));
            if(it == s_format_items.end()){
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                m_error = true;
            }else{
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
    }

}

LoggerManager::LoggerManager(const std::string& name) {
    m_root.reset(new Logger(name));
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender()));
   
    m_loggers[m_root->m_name] = m_root;
    init();
    }
Logger::ptr LoggerManager::getLogger(const std::string& name) {
    auto it = m_loggers.find(name);
    if(it != m_loggers.end()) {
        return it->second;
    }
    Logger::ptr logger(new Logger(name));
    logger->addAppender(StdoutLogAppender::ptr(new StdoutLogAppender()));
    // logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}

void LoggerManager::init(){}




}


