/*
 * @Author: closing-f fql2018@bupt.edu.cn
 * @Date: 2023-04-09 01:01:22
 * @LastEditors: closing
 * @LastEditTime: 2023-06-16 11:12:37
 * @FilePath: /sylar/src/config.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "config.h"
#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
namespace server_cc
{

static server_cc::Logger::ptr g_logger = SERVER_CC_LOG_NAME("system");

/**
 * @description: 用于将配置文件中的配置项转换为list<pair<string,YAML::Node>>的形式
 * @param {string&} prefix  配置项的前缀
 * @param {Node&} node  
 * @return {*}
 */
static void ListAllMember(const std::string& prefix, const YAML::Node& node, std::list<std::pair<std::string, const YAML::Node>>& output){
    
    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos){
        
        SERVER_CC_LOG_ERROR(g_logger) << "Config invalid name:" << prefix << " : " << node;
        return;
    }
    output.push_back(std::make_pair(prefix,node));
    if(node.IsMap()){
        for(auto it = node.begin();it != node.end();++it){
            ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(),it->second,output);
        }
    }
    if(node.IsSequence()){
        for(size_t i = 0;i < node.size();++i){
            ListAllMember(prefix + "." + std::to_string(i),node[i],output);
        }
    }
};

ConfigVarBase::ptr Config::LookupBase(const std::string& name){
    RWMutexType::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
};

void Config::LoadFromYaml(const YAML::Node& root){
    std::list<std::pair<std::string,const YAML::Node>> all_nodes;
    ListAllMember("",root,all_nodes);
    
    for(auto& i : all_nodes){
        std::string key = i.first;
        // std::cout<<key<<std::endl;
        if(key.empty()){
            continue;
        }
        std::transform(key.begin(),key.end(),key.begin(),::tolower);
        ConfigVarBase::ptr var = LookupBase(key);

        //? 如果配置文件中的配置项在程序中没有对应的配置项,则跳过
        if(var){
            if(i.second.IsScalar()){
                var->fromString(i.second.Scalar());
            }
            else{
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    
    }
};







}