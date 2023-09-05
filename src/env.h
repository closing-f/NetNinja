/*
 * @Author: closing
 * @Date: 2023-06-21 08:06:32
 * @LastEditors: closing
 * @LastEditTime: 2023-06-21 08:57:08
 * @Description: 从命令行中获取参数，以及环境变量（绝对工作目录）
 */
#ifndef ENV_H
#define ENV_H

#include "mutex.h"
#include "utils.h"
#include <map>
#include <vector>
namespace server_cc{
class Env {
typedef RWMutex RWMutexType;
public:
    bool init(int argc, char** argv);
    void add(const std::string& key,const std::string& val);
    bool has(const std::string& key);
    void del(const std::string& key);
    std::string get(const std::string& key,const std::string& default_value = "");
    void addHelp(const std::string& key,const std::string& desc);
    void removeHelp(const std::string& key);
    void printHelp();
    
    const std::string& getProgram() const {return m_program;}
    const std::string& getExe() const {return m_exe;}
    const std::string& getCwd() const {return m_cwd;}

    bool setEnv(const std::string& key,const std::string& val);
    std::string getEnv(const std::string& key,const std::string& default_value = "");

    std::string getAbsolutePath(const std::string& path) const;
    std::string getAbsoluteWorkPath(const std::string& path) const;
    std::string getConfigPath();

private:
    RWMutexType m_mutex;
    std::map<std::string, std::string> m_args;
    std::vector<std::pair<std::string,std::string>> m_helps;

    std::string m_program;//程序名称
    std::string m_exe;//可执行文件的绝对路径
    std::string m_cwd;//当前工作目录的绝对路径
};
typedef server_cc::Singleton<Env> EnvMgr;
}// namespace server_cc

#endif // ENV_H
