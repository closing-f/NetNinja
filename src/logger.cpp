#include "logger.h"
#include "config.h"
namespace server_cc{


class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};
class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os <<  LogLevel::ToString(level);
    }
};
class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};
class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLogger()->getName();
    }
};
class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};
class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
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
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
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
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile();
    }
};
class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};
class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};
class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str) : m_string(str) {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};
class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
    }
};
class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};





LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
            ,const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
            ,const std::string& thread_name)
    :m_filename(file)
    ,m_line(line)
    ,m_elapse(elapse)
    ,m_threadId(thread_id)
    ,m_fiberId(fiber_id)
    ,m_time(time)
    ,m_threadName(thread_name)
    ,m_logger(logger)
    ,m_level(level) {
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
/**
 * @description: 析构函数，调用log方法
 * @return {*}
 */
LogEventWrapper::~LogEventWrapper() {

    m_event->getLogger()->log(m_event->getLevel(), m_event);
}
std::stringstream& LogEventWrapper::getSS() {
    return m_event->getSS();
}


//调用appenders的log方法
void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        auto self = shared_from_this();
        MutexType::Lock lock(m_mutex);
        if(!m_appenders.empty()) {
            for(auto& i : m_appenders) {
                i->log(self, level, event);
            }
        } else if(m_root) {
            m_root->log(level, event);
        }
    }
}
void Logger::addAppender(LogAppender::ptr appender) {

    MutexType::Lock lock(m_mutex);
    if(!appender->getFormatter()) {
        MutexType::Lock ll(appender->m_mutex);
        appender->m_formatter = m_formatter;
    }
    m_appenders.push_back(appender);
    
}
void Logger::delAppender(LogAppender::ptr appender) {
    MutexType::Lock lock(m_mutex);
    for(auto it = m_appenders.begin();
            it != m_appenders.end(); ++it) {
        if(*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}
void Logger::clearAppenders() {
    MutexType::Lock lock(m_mutex);
    m_appenders.clear();
}

void Logger::setFormatter(LogFormatter::ptr val) {
    MutexType::Lock lock(m_mutex);
    m_formatter = val;

    for(auto& i : m_appenders) {
        MutexType::Lock ll(i->m_mutex);
        if(!i->m_hasFormatter) {
            i->m_formatter = m_formatter;
        }
    }
}
void Logger::setFormatter(const std::string& val) {
    std::cout << "---" << val << std::endl;
    server_cc::LogFormatter::ptr new_val(new server_cc::LogFormatter(val));
    if(new_val->isError()) {
        std::cout << "Logger setFormatter name=" << m_name
                  << " value=" << val << " invalid formatter"
                  << std::endl;
        return;
    }
    //m_formatter = new_val;
    setFormatter(new_val);
}
LogFormatter::ptr Logger::getFormatter() {
    MutexType::Lock lock(m_mutex);
    return m_formatter;
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

std::string Logger::ToYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["name"] = m_name;
    if(m_level != LogLevel::UNKNOWN) {
        node["level"] = LogLevel::getLevel(m_level);
    }
    if(m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
  
    for(auto& i : m_appenders) {
    
        node["appenders"].push_back(YAML::Load(i->ToYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
};

//Appender
void LogAppender::setFormatter(LogFormatter::ptr val) {
    MutexType::Lock lock(m_mutex);
    m_formatter = val;
    if(m_formatter) {
        m_hasFormatter = true;
    } else {
        m_hasFormatter = false;
    }
}

LogFormatter::ptr LogAppender::getFormatter() {
    MutexType::Lock lock(m_mutex);
    return m_formatter;
}

FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename) {
    reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        uint64_t now = event->getTime();
        if(now >= (m_lastTime + 3)) {
            reopen();
            m_lastTime = now;
        }
        MutexType::Lock lock(m_mutex);

        //formatter的format方法用来格式化日志
        if(!m_formatter->format(m_filestream, logger, level, event)) {
            std::cout << "error" << std::endl;
        }
    }
}

bool FileLogAppender::reopen() {
    MutexType::Lock lock(m_mutex);
    if(m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename.c_str(), std::ios::app);  //ios::app:所有输出附加到文件末尾
    //TODO Dirname
    // if(!m_filestream.is_open()) {
    //     std::string dir = Dirname(m_filename);
    //     Mkdir(dir);
    //     m_filestream.open(m_filename.c_str(), std::ios::app);
    // }
    return m_filestream.is_open();
}
 
std::string FileLogAppender::ToYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_filename;
    if(m_level != LogLevel::UNKNOWN) {
        node["level"] = LogLevel::getLevel(m_level);
    }
    if(m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
};

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        MutexType::Lock lock(m_mutex);
        m_formatter->format(std::cout, logger, level, event);
    }
}
std::string StdoutLogAppender::ToYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    if(m_level != LogLevel::UNKNOWN) {
        node["level"] = LogLevel::getLevel(m_level);
    }
    if(m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
};

//LogFormatter
LogFormatter::LogFormatter(const std::string& pattern) 
    : m_pattern(pattern) {
    init();
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for(auto& i : m_items) {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    for(auto& i : m_items) {
        i->format(ofs, logger, level, event);
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
        {"m", [](const std::string& fmt) { return FormatItem::ptr(new MessageFormatItem(fmt)); }}, //m:消息
        {"p", [](const std::string& fmt) { return FormatItem::ptr(new LevelFormatItem(fmt)); }},  //p: 日志级别
        {"r", [](const std::string& fmt) { return FormatItem::ptr(new ElapseFormatItem(fmt)); }}, //r: 累计毫秒数
        {"c", [](const std::string& fmt) { return FormatItem::ptr(new NameFormatItem(fmt)); }}, //c: 日志名称
        {"t", [](const std::string& fmt) { return FormatItem::ptr(new ThreadIdFormatItem(fmt)); }}, //t: 线程id
        {"n", [](const std::string& fmt) { return FormatItem::ptr(new NewLineFormatItem(fmt)); }}, //n:换行
        {"d", [](const std::string& fmt) { return FormatItem::ptr(new DateTimeFormatItem(fmt)); }},//d:时间
        {"f", [](const std::string& fmt) { return FormatItem::ptr(new FilenameFormatItem(fmt)); }},//f：文件名
        {"l", [](const std::string& fmt) { return FormatItem::ptr(new LineFormatItem(fmt)); }},//l:行号
        {"F", [](const std::string& fmt) { return FormatItem::ptr(new FiberIdFormatItem(fmt)); }},//f: 文件名
        {"N", [](const std::string& fmt) { return FormatItem::ptr(new ThreadNameFormatItem(fmt)); }},//N: 线程名称
        {"T", [](const std::string& fmt) { return FormatItem::ptr(new TabFormatItem(fmt)); }}, //tab
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
    m_loggers[m_root->getName()] = m_root;
}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    MutexType::Lock lock(m_mutex);
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




struct LogAppenderDefine{
    std::string type;//0 file 1 stdout
    std::string file = "";//日志文件路径
    LogLevel::Level level = LogLevel::Level::DEBUG;
    std::string format = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%f%T[%p]%T[%c]:%T%m %n";
    bool has_file = false;
    bool operator==(const LogAppenderDefine& oth) const {
        return type == oth.type
            && file == oth.file
            && level == oth.level
            && format == oth.format
;
    }
};

struct LogDefine{
    std::string name;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;
    
    LogLevel::Level level = LogLevel::Level::DEBUG;

    bool operator==(const LogDefine& oth) const {
        return name == oth.name
            && formatter == oth.formatter
            && appenders == oth.appenders
            && level == oth.level;
    };
    bool operator<(const LogDefine& oth) const {
        return name < oth.name;
    }
    
};



//该类没有模板参数
template<>
class LexicalCast<std::string, LogDefine> {
    public:
        LogDefine operator()(const std::string& v){
            YAML::Node node = YAML::Load(v);

            LogDefine log;
            
            if(node["name"]){
                log.name = node["name"].as<std::string>();
            }else{
                 std::cout << "log config error: name is null, " << node
                      << std::endl;
                return log;
            }
            log.level= LogLevel::FromString(node["level"].IsDefined()? node["level"].as<std::string>() : "DEBUG");//如果没有定义level，就默认为DEBUG
            
            
            if(node["formatter"]){
                log.formatter = node["formatter"].as<std::string>();
            }
            
            if(node["appenders"].IsDefined()){
                for(size_t a=0;a<node["appenders"].size();a++){
                    YAML::Node a_node = node["appenders"][a];
                    //如果type存在,0 file 1 stdout
        
                    std::string type = a_node["type"].as<std::string>();
                   
                    LogAppenderDefine lad;
                    
                    if(type=="FileLogAppender"){
                        
                        
                        lad.type = a_node["type"].as<std::string>();
                        
                        lad.file = a_node["file"].as<std::string>();
                        lad.has_file = true;
                        
                        // if(a_node["level"].IsDefined()){
                        //     lad.level = LogLevel::FromString(a_node["level"].as<std::string>());
                        // }
                        // if(a_node["format"].IsDefined()){

                        //     lad.format = a_node["format"].as<std::string>();

                        // }
                        // std::cout<<"FileLogAppender"<<std::endl;
                        log.appenders.push_back(lad);
                    }else if(type=="StdoutLogAppender"){
                        lad.type = a_node["type"].as<std::string>();
                        // std::cout<<"StdoutLogAppender"<<std::endl;
                        log.appenders.push_back(lad);
                    }
                }
            }

            return log;

        };

};

template<>
class LexicalCast<LogDefine,std::string>{
    public:
        std::string operator()(const LogDefine& v){
            YAML::Node node;
            node["name"] = v.name;
            if(v.level != LogLevel::Level::DEBUG){
                node["level"] = LogLevel::ToString(v.level);
            }
            if(!v.formatter.empty()){
                node["formatter"] = v.formatter;
            }
            for(auto& i : v.appenders){
                YAML::Node a_node;
                a_node["type"] = i.type;
                if(i.level != LogLevel::Level::DEBUG){
                    a_node["level"] = LogLevel::ToString(i.level);
                }
                if(i.has_file){
                    a_node["file"] = i.file;
                }
                if(!i.format.empty()){
                    a_node["format"] = i.format;
                }
                node["appenders"].push_back(a_node);
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
};
//包含多个LogDefine的容器
server_cc::ConfigVar<std::set<LogDefine>>::ptr g_log_defintion = 
    server_cc::Config::Lookup("logs",std::set<LogDefine>(),"logs config");

struct LogIniter {
    LogIniter(){
        g_log_defintion->addListener([](const std::set<LogDefine>& old_value,const std::set<LogDefine>& new_value){
            
            
            for(auto& i : new_value){
                auto x = old_value.find(i);
                if(x == old_value.end()){
                    //新增logger
                    auto logger = LoggerMgr::GetInstance().getLogger(i.name);
                    logger->clearAppenders();
                    logger->setLevel(i.level);
                    if(!i.formatter.empty()){
                        logger->setFormatter(i.formatter);
                    }
                    for(auto& a : i.appenders){
                        LogAppender::ptr ap;
                        if(a.type == "FileLogAppender"){
                            ap.reset(new FileLogAppender(a.file));
                        }else{
                            ap.reset(new StdoutLogAppender());
                        }
                        ap->setLevel(a.level);
                        
                        
    
                        if(a.format!=""){
                            LogFormatter::ptr fmt(new LogFormatter(a.format));
                            if(!fmt->isError()) {
                            ap->setFormatter(fmt);
                        } else {
                            std::cout << "log.name=" << i.name << " appender type=" << a.type
                                      << " formatter=" << a.format << " is invalid" << std::endl;
                        }
                        }
                        
                        logger->addAppender(ap);
                    }
                }else{
                    //修改logger
                    if(!(i == *x)){
                        auto logger = LoggerMgr::GetInstance().getLogger(i.name);
                        logger->setLevel(i.level);
                        if(!i.formatter.empty()){
                            logger->setFormatter(i.formatter);
                        }
                        logger->clearAppenders();
                        for(auto& a : i.appenders){
                            LogAppender::ptr ap;
                            if(a.type == "FileLogAppender"){
                                ap.reset(new FileLogAppender(a.file));
                            }else{
                                ap.reset(new StdoutLogAppender());
                            }
                            ap->setLevel(a.level);
                            if(!a.format.empty()){

                                LogFormatter::ptr fmt(new LogFormatter(a.format));
                                if(!fmt->isError()) {
                                    ap->setFormatter(fmt);
                                } else {
                                    std::cout << "log.name=" << i.name << " appender type=" << a.type
                                            << " formatter=" << a.format << " is invalid" << std::endl;
                                }
                            }
                            logger->addAppender(ap);
                        }
                    }
                }
            }

        });
    }
};
static LogIniter __log_init;
std::string LoggerManager::ToYamlString(){
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for(auto& i : m_loggers){
   
        node.push_back(YAML::Load(i.second->ToYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();

};






}




