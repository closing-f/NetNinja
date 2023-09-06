<!--
 * @Author: closing
 * @Date: 2023-09-02 15:37:18
 * @LastEditors: closing
 * @LastEditTime: 2023-09-06 09:04:28
 * @Description: 请填写简介
-->
<font size=5>

## 1.日志模块(/src/logger.h)
支持多日志分离,写入多种输出端；支持日志格式自定义；支持流式日志风格写日志和格式化风格写日志：
流式日志使用：SERVER_CC_LOG_INFO(logger) << "C++ Webserver";
格式化日志使用：SERVER_CC_LOG_INFO(logger, "%s", "C++ Webserver");
类关系图：
![image](/picture/log.svg)

## 2.配置模块(/src/config.h)
使用YAML文件做为配置内容。定义即可使用,不需要单独去解析；
支持变更通知功能，当配置修改重新加载，该值自动更新
支持级别格式的数据类型，支持STL容器(vector,list,set,map等等),支持自定义类型的支持（需要将序列化和反序列化模板具现化)使用方式如下：
```sh
system:
      port: 9900
```
在yaml文件定义了一个名为system.port配置变量，可以通过如下方法, 直接使用 g_system_port->getValue() 获取参数的值，
```cpp
static server_cc::ConfigVar<int>::ptr g_system_port =
	server_cc::Config::Lookup("system.port", 9000, "system port");
```
## 3.线程模块
使用pthread线程,封装了线程中需要用的Thread,Semaphore,Mutex等对象
## 4.协程模块
协程：用户态的线程，更轻量级。基于ucontext_t来实现的。后续通过配置socket hook，可以把复杂的异步调用，封装成同步操作。降低业务逻辑的编写复杂度。类图如下：
![image](/picture/fiber.svg)
## 5.协程调度模块
协程调度器，管理协程的调度，内部实现为一个线程池，支持协程在多线程中切换，也可以指定协程在固定的线程中执行。是一个N-M的协程调度模型，N个线程，M个协程。重复利用每一个线程。类图如下：
![image](/picture/scheduler.svg)

其中scheduler中的主要运行逻辑如下：
![image](/picture/scheduler_run.svg)

## 6.IO协程调度模块
继承与协程调度器，封装了epoll（Linux），并支持定时器功能（使用epoll实现定时器，精度毫秒级）,支持Socket读写时间的添加，删除，取消功能。支持一次性定时器，循环定时器，条件定时器等功能
epoll函数运行在scheduler的idle函数中，在空闲状态时监听IO事件，运行逻辑如下：
![image](/picture/scheduler_idle.svg)
## 7.Hook模块
hook系统底层和socket相关的API，socket io相关的API，以及sleep系列的API。hook的开启控制是线程粒度的。可以自由选择。通过hook模块，可以使一些不具异步功能的API，展现出异步的性能。如（mysql）
## 8.Socket模块
封装了Socket类，提供所有socket API功能，统一封装了地址类，将IPv4，IPv6，Unix地址统一起来。并且提供域名，IP解析功能。类图如下：
![image](/picture/socket.svg)
## 9.ByteArray序列化模块
ByteArray二进制序列化模块，提供对二进制数据的常用操作。读写入基础类型int8_t,int16_t,int32_t,int64_t等，支持Varint,std::string的读写支持,支持字节序转化,支持序列化到文件，以及从文件反序列化等功能
## 10.TcpServer模块
基于Socket类，封装了一个通用的TcpServer的服务器类，提供简单的API，使用便捷，可以快速绑定一个或多个地址，启动服务，监听端口，accept连接，处理socket连接等功能。具体业务功能更的服务器实现，只需要继承该类就可以快速实现,类图如下：
![image](/picture/tcpserver.svg)
## 11.Stream模块
封装流式的统一接口。将文件，socket封装成统一的接口。使用的时候，采用统一的风格操作。基于统一的风格，可以提供更灵活的扩展。目前实现了SocketStream。
## 12.HTTP模块
采用Ragel（有限状态机），实现了HTTP/1.1的简单协议实现和uri的解析。基于SocketStream实现了HttpConnection(HTTP的客户端)和HttpSession(HTTP服务器端的链接）。基于TcpServer实现了HttpServer。提供了完整的HTTP的客户端API请求功能，HTTP基础API服务器功能，类关系图如下：

![image](/picture/stream.svg)
## 13.Servlet模块
仿照java的servlet，实现了一套Servlet接口，实现了ServletDispatch，FunctionServlet。NotFoundServlet。支持uri的精准匹配，模糊匹配等功能。和HTTP模块，一起提供HTTP服务器功能

