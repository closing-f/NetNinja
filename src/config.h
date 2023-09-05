#ifndef CONFIG_H
#define CONFIG_H
#include <memory>
#include<algorithm>
#include<yaml-cpp/yaml.h>
#include "boost/lexical_cast.hpp"
#include<map>
#include<list>
#include<set>
#include<unordered_map>
#include<unordered_set>
#include<typeinfo>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "logger.h"
#include "utils.h"
#include "thread.h"
namespace server_cc{
    
using boost::lexical_cast;
using boost::bad_lexical_cast;
//ConfigVar的基类
class ConfigVarBase{
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        //由于无法判断具体类型，所以只能用字符串来表示name和description
        ConfigVarBase(const std::string& name, const std::string& description="")
            :m_name(name)
            ,m_description(description)
        {   
            //将name转换为小写,方便查找
            std::transform(m_name.begin(),m_name.end(),m_name.begin(),::tolower);
        }

        virtual ~ConfigVarBase() {}
        const std::string& getName() const { return m_name; }
        const std::string& getDescription() const { return m_description; }
        
        virtual std::string toString()=0;
        virtual bool fromString(const std::string& val)=0;
        
        virtual std::string getTypeName() const=0;
    protected:
        std::string m_name;
        std::string m_description;
};
template<class F, class T>
class LexicalCast {
public:
    /**
     * @brief 类型转换
     * @param[in] v 源类型值
     * @return 返回v转换后的目标类型
     * @exception 当类型不可转换时抛出异常
     */
    T operator()(const F& v) {
        return lexical_cast<T>(v);
    }
};
template<class T>
class LexicalCast<std::string, std::vector<T> > {
public:
    std::vector<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::vector<T> 转换成 YAML String)
 */
template<class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::list<T>)
 */
template<class T>
class LexicalCast<std::string, std::list<T> > {
public:
    std::list<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::list<T> 转换成 YAML String)
 */
template<class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::set<T>)
 */
template<class T>
class LexicalCast<std::string, std::set<T> > {
public:
    std::set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::set<T> 转换成 YAML String)
 */
template<class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_set<T>)
 */
template<class T>
class LexicalCast<std::string, std::unordered_set<T> > {
public:
    std::unordered_set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::unordered_set<T> 转换成 YAML String)
 */
template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::map<std::string, T>)
 */
template<class T>
class LexicalCast<std::string, std::map<std::string, T> > {
public:
    std::map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin();
                it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::map<std::string, T> 转换成 YAML String)
 */
template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_map<std::string, T>)
 */
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T> > {
public:
    std::unordered_map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin();
                it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::unordered_map<std::string, T> 转换成 YAML String)
 */
template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @description: 配置参数类
 * @return {*}
 */
template<class T,class FromStr = LexicalCast<std::string, T>
                ,class ToStr = LexicalCast<T, std::string> >
class ConfigVar : public ConfigVarBase{
    public:
        typedef RWMutex RWMutexType;
        typedef std::shared_ptr<ConfigVar> ptr;
        typedef std::function<void(const T& old_value,const T& new_value)> on_change_cb;
        
        /**
         * @description: 通过参数名,参数值,描述构造ConfigVar
         * @param {string&} name 参数名称有效字符为[0-9a-z_.]
         * @param {T&} default_value 参数的默认值
         * @param {string&} description 参数的描述
         * @return {*}
         */        
        ConfigVar(const std::string& name,const T& default_value,const std::string& description="")
            :ConfigVarBase(name,description)
            ,m_val(default_value)
        {
        }


        std::string toString() override{
            try{
                //第一个()表示ToStr构造函数，第二个()表示调用ToStr的operator()函数
                RWMutexType::ReadLock lock(m_mutex);
                return ToStr()(m_val);
                
            }
            catch (bad_lexical_cast &e) {
                
                return "";
            }
            
        }
        bool fromString(const std::string& val) override{
            try{
                //as<T>()将字符串转换为T类型，如果转换失败，会抛出异常
                setValue(FromStr()(val));

                return true;
            }
            catch (bad_lexical_cast &e) {
                
                return false;
            }
        }
        std::string getTypeName() const override { return typeid(T).name(); }
        const T getValue() const {
            RWMutexType::ReadLock lock(m_mutex); 
            return m_val; }
        
        void setValue(const T& v){
            {
                RWMutexType::ReadLock lock(m_mutex);
                if(m_val == v){
                    return;
                }
                //遍历回调函数
                for(auto& i : m_cbs){
                    i.second(m_val,v);
                }
            }
            RWMutexType::ReadLock lock(m_mutex);
            m_val = v;
        }
        void addListener(on_change_cb cb){
            static u_int32_t key = 0;
            RWMutexType::WriteLock lock(m_mutex);
            key++;
            m_cbs[key] = cb;
        }
        void deleteListener(u_int32_t key){
            RWMutexType::WriteLock lock(m_mutex);
            m_cbs.erase(key);
        }
        void clearListener(){
            RWMutexType::WriteLock lock(m_mutex);
            m_cbs.clear();
        }
        /**
         * @brief 获取回调函数
         * @param[in] key 回调函数的唯一id
         * @return 如果存在返回对应的回调函数,否则返回nullptr
         */
        on_change_cb getListener(uint64_t key) {
        RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }
        T& getValue() { return m_val; }
        
    private:
        RWMutexType m_mutex;
        T m_val;
        std::map<u_int32_t,on_change_cb> m_cbs;
};

/**
 * @description: 提供查找和创建ConfigVar<T>的接口
 * @return {*}
 */
class Config{
        typedef std::map<std::string,ConfigVarBase::ptr> ConfigVarMap;
        typedef std::list<std::pair<std::string,std::string>> ConfigVarList;
        typedef std::shared_ptr<Config> ptr;
        typedef RWMutex RWMutexType;
        
        public:
        /**
         * @description: 如果存在就返回，不存在就创建
         * @return {*} 返回ConfigVar<T>::ptr
         */        
        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string& name,const T& default_value,const std::string& description=""){
            RWMutexType::WriteLock lock(GetMutex());
            std::string tmp_name = name;
            std::transform(tmp_name.begin(),tmp_name.end(),tmp_name.begin(),::tolower);//将name转换为小写
            //查找name是否存在
            auto it = GetDatas().find(tmp_name);
            if(it != GetDatas().end()){
                //存在，判断类型是否一致
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
                if(tmp){
                    SERVER_CC_LOG_INFO(SERVER_CC_LOG_ROOT()) << "Lookup name=" << name << " exists";
                    return tmp;
                }
                else{
                    SERVER_CC_LOG_ERROR(SERVER_CC_LOG_ROOT()) << "Lookup name=" << name << " exists but type not "
                        << typeid(T).name() << " real_type= " << it->second->getTypeName()
                        << " " << it->second->toString();
                    return nullptr;
                }
            }
            //对名称的命名进行检查
            if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos){
                SERVER_CC_LOG_ERROR(SERVER_CC_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }
            //如果不存在，就创建一个
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(tmp_name,default_value,description));
            //将name和ConfigVar<T>插入到map中,静态函数中的变量是全局的，所以不会被销毁
            GetDatas()[tmp_name] = v;
            return v;
        }
        
         /**
         * @description: 查找name是否存在，不存在返回nullptr
         * @return {*} 返回ConfigVar<T>::ptr
         */ 
        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string& name){
            RWMutexType::ReadLock lock(GetMutex());
            //将name转换为小写
            std::string tmp_name = name;
            std::transform(tmp_name.begin(),tmp_name.end(),tmp_name.begin(),::tolower);
            //查找name是否存在
            auto it = GetDatas().find(tmp_name);
            if(it == GetDatas().end()){
                return nullptr;
            }
            
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);//将基类的指针或引用安全地转换成派生类的指针或引用，并用派生类的指针或引用调用非虚函数
        }
        

    static ConfigVarMap& GetDatas(){
        static ConfigVarMap s_datas;
        return s_datas;
    }

    /**
     * @description: 将配置文件中的配置项转换为字符串
     * @param {Node&} root
     * @return {*}
     */    
    static void LoadFromYaml(const YAML::Node& root);

    /**
     * @brief 加载path文件夹里面的配置文件
     */
    static void LoadFromConfDir(const std::string& path, bool force = false);

    /**
     * @description: 查找name是否存在，不存在返回nullptr，不会创建
     * @param {string&} name
     * @return {*}
     */    
    static ConfigVarBase::ptr LookupBase(const std::string& name);

    /**
     * @brief 遍历配置模块里面所有配置项
     * @param[in] cb 配置项回调函数
     */
    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);
    /**
     * @brief 配置项的RWMutex
     */
    static RWMutexType& GetMutex() {
        static RWMutexType s_mutex;
        return s_mutex;
    }

};


}
#endif // !

