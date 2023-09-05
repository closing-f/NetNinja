#include <iostream>
#include "src/logger.h"

// #include "server_cc/util.h"
#include<string>
//dsad 
int main() {
    server_cc::Logger::ptr logger(new server_cc::Logger);
    server_cc::LogAppender::ptr stdout_appender(new server_cc::StdoutLogAppender);
    
    
    server_cc::LogAppender::ptr file_appender(new server_cc::FileLogAppender("./log.txt"));
    
    // file_appender->setFormatter(fmt);
    // file_appender->setLevel(server_cc::LogLevel::DEBUG);
    
    // stdout_appender->setFormatter(fmt);
    // stdout_appender->setLevel(server_cc::LogLevel::DEBUG);
    logger->addAppender(file_appender);
    logger->addAppender(stdout_appender);
    
   
   
    // server_cc::LogEvent::ptr event(new server_cc::LogEvent());
    //event->getSS() << "hello server_cc log";
    // logger->log(server_cc::LogLevel::DEBUG, event);

    SEVER_CC_LOG_DEBUG(logger) << "test macro";
    SEVER_CC_LOG_DEBUG(logger) << "test macro error";
    SEVER_CC_LOG_FMT_DEBUG(logger, "test macro fmt %s", "aa");
    
    auto l = server_cc::LoggerMgr::GetInstance().getLogger("xx");
    // SYLAR_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

    SEVER_CC_LOG_DEBUG(SEVER_CC_LOG_ROOT()) << "test macro";
    SEVER_CC_LOG_INFO(l) << "xxx";
    return 0;
}