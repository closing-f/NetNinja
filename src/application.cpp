#include "application.h"

#include <unistd.h>
#include <signal.h>

#include "tcpserver.h"
#include "daemon.h"
#include "config.h"
#include "env.h"
#include "logger.h"

namespace server_cc {

static server_cc::Logger::ptr g_logger = SERVER_CC_LOG_NAME("system");

static server_cc::ConfigVar<std::string>::ptr g_server_work_path =
    server_cc::Config::Lookup("server.work_path"
            ,std::string("/home/fql/Project/C++/sylar/src/workdir")
            , "server work path");

static server_cc::ConfigVar<std::string>::ptr g_server_pid_file =
    server_cc::Config::Lookup("server.pid_file"
            ,std::string("server_cc.pid")
            , "server pid file");

struct HttpServerConf{
    std::vector<std::string> address;
    int keepalive = 0;
    int timeout = 1000 * 2 * 60;
    std::string name;

    bool isValid() const {
        return !address.empty();
    }

    bool operator == (const HttpServerConf& oth) const {
        return address == oth.address
            && keepalive == oth.keepalive
            && timeout == oth.timeout
            && name == oth.name;
    }

};

template<>
class LexicalCast<std::string, HttpServerConf>{
public:
    HttpServerConf operator()(const std::string& v){
        YAML::Node node = YAML::Load(v);
        HttpServerConf conf;
        if(node["address"].IsDefined()){
            for(size_t i = 0;i < node["address"].size();++i){
                conf.address.push_back(node["address"][i].as<std::string>());
            }
            
        }
        
        conf.keepalive = node["keepalive"].as<int>();
        conf.timeout = node["timeout"].as<int>();
        conf.name = node["name"].as<std::string>();
        return conf;
    }
};

template<>
class LexicalCast<HttpServerConf, std::string>{
public:
    std::string operator()(const HttpServerConf& conf){
        YAML::Node node;
        
        node["keepalive"] = conf.keepalive;
        node["timeout"] = conf.timeout;
        node["name"] = conf.name;
        for(auto& i : conf.address){
            node["address"].push_back(i);
        } 
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};



static server_cc::ConfigVar<std::vector<HttpServerConf> >::ptr g_servers_conf
    = server_cc::Config::Lookup("servers", std::vector<HttpServerConf>(), "http server config");

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;
}

bool Application::init(int argc, char** argv) {
    m_argc = argc;
    m_argv = argv;

    server_cc::EnvMgr::GetInstance().addHelp("s", "start with the terminal");
    server_cc::EnvMgr::GetInstance().addHelp("d", "run as daemon");
    server_cc::EnvMgr::GetInstance().addHelp("c", "conf path default: ./conf");
    server_cc::EnvMgr::GetInstance().addHelp("p", "print help");

    bool is_print_help = false;
    if(!server_cc::EnvMgr::GetInstance().init(argc, argv)) {
        is_print_help = true;
    }

    if(server_cc::EnvMgr::GetInstance().has("p")) {
        is_print_help = true;
    }

    std::string conf_path = server_cc::EnvMgr::GetInstance().getConfigPath();
    SERVER_CC_LOG_INFO(g_logger) << "load conf path:" << conf_path;
    server_cc::Config::LoadFromConfDir(conf_path);

    // ModuleMgr::GetInstance().init();//初始化模块管理器
    // std::vector<Module::ptr> modules;
    // ModuleMgr::GetInstance().listAll(modules);

    // for(auto i : modules) {
    //     i->onBeforeArgsParse(argc, argv);
    // }

    if(is_print_help) {
        server_cc::EnvMgr::GetInstance().printHelp();
        // return false;
    }

    // for(auto i : modules) {
    //     i->onAfterArgsParse(argc, argv);
    // }
    // modules.clear();

    int run_type = 0;
    if(server_cc::EnvMgr::GetInstance().has("s")) {
        run_type = 1;
    }
    if(server_cc::EnvMgr::GetInstance().has("d")) {
        run_type = 2;
    }

    if(run_type == 0) {
        server_cc::EnvMgr::GetInstance().printHelp();
        //TODO 记得添加下面
        // return false;
    }

    std::string pidfile = g_server_work_path->getValue()
                                + "/" + g_server_pid_file->getValue();
    if(server_cc::FSUtil::IsRunningPidfile(pidfile)) {
        SERVER_CC_LOG_ERROR(g_logger) << "server is running:" << pidfile;
        return false;
    }

    if(!server_cc::FSUtil::Mkdir(g_server_work_path->getValue())) {
        SERVER_CC_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue()
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    // std::ofstream ofs(pidfile);
    // if(!ofs) {
    //     SERVER_CC_LOG_ERROR(g_logger) << "open pidfile " << pidfile << " failed";
    //     return false;
    // }
    // ofs << getpid();
    return true;
}

bool Application::run() {
    bool is_daemon = server_cc::EnvMgr::GetInstance().has("d");
    return start_daemon(m_argc, m_argv,
            std::bind(&Application::main, this, std::placeholders::_1,
                std::placeholders::_2), is_daemon);
}

int Application::main(int argc, char** argv) {
    // signal(SIGPIPE, SIG_IGN);
    SERVER_CC_LOG_INFO(g_logger) << "main";
    // std::string conf_path = server_cc::EnvMgr::GetInstance().getConfigPath();
    // server_cc::Config::LoadFromConfDir(conf_path, true);

    std::string pidfile = g_server_work_path->getValue()
                                + "/" + g_server_pid_file->getValue();
    std::ofstream ofs(pidfile);
    if(!ofs) {
        SERVER_CC_LOG_ERROR(g_logger) << "open pidfile " << pidfile << " failed";
        return false;
    }
    ofs << getpid();
    

    

    m_mainIOManager.reset(new server_cc::IOManager(1, true, "main"));
    m_worker.reset(new server_cc::IOManager(3, false, "worker"));
    m_mainIOManager->schedule(std::bind(&Application::run_fiber, this));
    // m_mainIOManager->addTimer(2000, [](){
    //         //SYLAR_LOG_INFO(g_logger) << "hello";
    // }, true);
    m_mainIOManager->stop();

    return 0;
}

int Application::run_fiber() {
    std::cout << "run_fiber" << std::endl;
    auto http_confs = g_servers_conf->getValue();
    for (auto& i : http_confs) {
        SERVER_CC_LOG_INFO(g_logger) << std::endl << LexicalCast<HttpServerConf, std::string>()(i);

        std::vector<Address::ptr> address;


        for(auto& a : i.address) {
            
            size_t pos = a.find(":");
            if(pos == std::string::npos) {
                SERVER_CC_LOG_ERROR(g_logger) << "invalid address1: " << a;
                continue;
            }
            auto addr = server_cc::Address::LookupAny(a);
            if(addr) {
                address.push_back(addr);
                continue;
            }

            std::vector<std::pair<Address::ptr, uint32_t> > result;
            if(!server_cc::Address::GetInterfaceAddresses(result, a.substr(0, pos))) {
                SERVER_CC_LOG_ERROR(g_logger) << "invalid address2: " << a;
                continue;
            }
            
            
            for(auto& x : result) {
                auto ipaddr = std::dynamic_pointer_cast<IPAddress>(x.first);
                if(ipaddr) {
                    ipaddr->setPort(atoi(a.substr(pos + 1).c_str()));
                }
                address.push_back(ipaddr);
            }

            
            
        }

        server_cc::http::HttpServer::ptr server(new server_cc::http::HttpServer(i.keepalive,m_worker.get()));
        std::vector<Address::ptr> fails;
        if(!server->bind(address, fails)) {
            for(auto& x : fails) {
                SERVER_CC_LOG_ERROR(g_logger) << "bind address fail: " << *x;
            }
            _exit(0);
        }
        std::cout<<"server->bind(address, fails)"<<std::endl;
        server->setName(i.name);
        server->start();
        m_httpservers.push_back(server);
        // worker.reset(new server_cc::IOManager(3, false, "worker"));
        
        // m_httpservers.push_back(server);
    }
    return 0;

}
}//namespace server_cc
