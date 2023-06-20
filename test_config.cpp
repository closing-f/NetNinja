
#include "src/config.h"
#include "src/logger.h"
#include"src/utils.h"
#include <yaml-cpp/yaml.h>

#include <iostream>


server_cc::ConfigVar<int>::ptr g_int_value_config =
    server_cc::Config::Lookup("system.port", (int)8080, "system port");

server_cc::ConfigVar<float>::ptr g_int_valuex_config =
    server_cc::Config::Lookup("system.port", (float)8080, "system port");

server_cc::ConfigVar<float>::ptr g_float_value_config =
    server_cc::Config::Lookup("system.value", (float)10.2f, "system value");

server_cc::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config =
    server_cc::Config::Lookup("system.int_vec", std::vector<int>{1,2}, "system int vec");

server_cc::ConfigVar<std::list<int> >::ptr g_int_list_value_config =
    server_cc::Config::Lookup("system.int_list", std::list<int>{1,2}, "system int list");

server_cc::ConfigVar<std::set<int> >::ptr g_int_set_value_config =
    server_cc::Config::Lookup("system.int_set", std::set<int>{1,2}, "system int set");

server_cc::ConfigVar<std::unordered_set<int> >::ptr g_int_uset_value_config =
    server_cc::Config::Lookup("system.int_uset", std::unordered_set<int>{1,2}, "system int uset");

server_cc::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config =
    server_cc::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k",2}}, "system str int map");

server_cc::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config =
    server_cc::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k",2}}, "system str int map");

void print_yaml(const YAML::Node& node, int level) {
    if(node.IsScalar()) {
        SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << std::string(level * 4, ' ')
            << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if(node.IsNull()) {
        SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << std::string(level * 4, ' ')
            << "NULL - " << node.Type() << " - " << level;
    } else if(node.IsMap()) {
        for(auto it = node.begin();
                it != node.end(); ++it) {
            SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << std::string(level * 4, ' ')
                    << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if(node.IsSequence()) {
        for(size_t i = 0; i < node.size(); ++i) {
            SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << std::string(level * 4, ' ')
                << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("/home/fql/Project/C++/sylar/log.yml");
    // print_yaml(root, 0);
    SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << root.Scalar();

    SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << root["test"].IsDefined();
    SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << root["logs"].IsDefined();
    SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << root;
}

void test_config() {
    SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << "before: " << g_float_value_config->toString();

#define XX(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << #prefix " " #name ": " << i; \
        } \
        SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << #prefix " " #name ": {" \
                    << i.first << " - " << i.second << "}"; \
        } \
        SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }


    // XX(g_int_vec_value_config, int_vec, before);
    
    // XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    // XX(g_int_uset_value_config, int_uset, before);
    // XX_M(g_str_int_map_value_config, str_int_map, before);
    // XX_M(g_str_int_umap_value_config, str_int_umap, before);

    YAML::Node root = YAML::LoadFile("/home/fql/Project/C++/sylar/log.yml");
    server_cc::Config::LoadFromYaml(root);
    
    SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << "after: " << g_float_value_config->toString();
    XX(g_int_set_value_config, int_set, after);
    // XX_M(g_str_int_umap_value_config, str_int_umap, before);


}
void test_log() {
    server_cc::Logger::ptr system_log = SERVER_CC_LOG_NAME("system");
    SERVER_CC_LOG_INFO(system_log) << "hello system" << std::endl;
    
    // std::cout << server_cc::LoggerMgr::GetInstance().ToYamlString() << std::endl;
    
    YAML::Node root = YAML::LoadFile("/home/fql/Project/C++/sylar/log.yml");
    server_cc::Config::LoadFromYaml(root);
    std::cout <<std::endl<< "=============" << std::endl;
    std::cout << server_cc::LoggerMgr::GetInstance().ToYamlString() << std::endl;
    // std::cout << "=============" << std::endl;
    // std::cout << root << std::endl;
    SERVER_CC_LOG_INFO(system_log) << "hello system" << std::endl;

    // system_log->setFormatter((std::string("%d - %m%n")));
    // SERVER_CC_LOG_INFO(system_log) << "hello system" << std::endl;
}

int main(int argc, char** argv) {
    // test_yaml();

    test_config();
    //test_class();
    // test_log();
    
    // test_loadconf();
    // std::cout << " ==== " << std::endl;
    // sleep(10);
    // test_loadconf();
    // return 0;
    // server_cc::Config::Visit([](server_cc::ConfigVarBase::ptr var) {
    //     SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << "name=" << var->getName()
    //                 << " description=" << var->getDescription()
    //                 << " typename=" << var->getTypeName()
    //                 << " value=" << var->toString();
    // });

    return 0;
}
