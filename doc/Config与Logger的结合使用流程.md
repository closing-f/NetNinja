Config与Logger的结合使用流程

1. 通过Lookup函数生成name为logs，value为set<LogDefine>的ConfigVar并自动添加到静态ConfigVarMap中

   ```c++
   //src/logger.cpp
   server_cc::ConfigVar<std::set<LogDefine>>::ptr g_log_defintion = 
       server_cc::Config::Lookup("logs",std::set<LogDefine>(),"logs config");
   ```

​			此时系统包含一个名字为logs的空set<LogDefine>

2. 通过LogIniter为1.中定义的g_log_defintion添加监听函数，当值改变时触发

   ```cpp
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
   
                               ap->setFormatter(a.format);
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
                                   ap->setFormatter(a.format);
                               }
                               logger->addAppender(ap);
                           }
                       }
                   }
               }
   
           });
       }
   ```

3. 通过YAML::LoadFile加载yml文件，通过Config::LoadFromYaml添加到系统中，yml文件如下：

   ```yaml
   logs:
       - name: root
         level: INFO
         appenders:
             - type: FileLogAppender
               file: /home/fql/Project/C++/sylar/log.txt
             - type: StdoutLogAppender
       - name: system
         level: INFO
         appenders:
             - type: FileLogAppender
               file: /home/fql/Project/C++/sylar/log.txt
             - type: StdoutLogAppender
   ```

   ```cpp
   YAML::Node root = YAML::LoadFile("/home/fql/Project/C++/sylar/log.yml");
       server_cc::Config::LoadFromYaml(root);
   ```

   LoadFromYaml函数如下：

   ```cpp
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
   
           if(var){
               // std::cout<<i.second<<std::endl;
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
   ```

   会通过LookupBase找到名为logs的ConfigVar，调用其fromstring方法，将yml文件的与logs相关的node通过string的形式传入

   

   fromString函数如下：

   ```cpp
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
   ```

   

   调用模板类FromStr()即 LexicalCast<std::string, T>，将string转为T,然后调用setValue触发监听函数，其中FromStr()会发生两次转换：

   **首先调用LexicalCast<std::string, std::set<T> > ，在其中根据yml中logs对应的项大小依次调用LexicalCast<std::string, LogDefine>，最终生成set<LogDefine>**

   

   最后调用监听函数，替换LogDefine级别的项，根据LogDefine中的name在系统中查找相应日志logger,如下所示

   ```cpp
   auto logger = LoggerMgr::GetInstance().getLogger(i.name);
   ```

   删除logger之前的appenders，根据yml文件配置重设level ,formatter,和appenders

​			

