# Linux_shell_chat
LINUX网络编程部分开源项目学习和改进
## 一、基于C++和POLL的服务端和客户端
&emsp;&emsp;1）文献来源：《Linux高性能服务器编程》——游双著；\
&emsp;&emsp;2）参考学习：https://blog.csdn.net/liuxuejiang158blog/article/details/12503269 \
&emsp;&emsp;3）使用说明：终端输入./server ip port即可运行服务端；终端输入./client ip port即可运行客户端；\
&emsp;&emsp;4）项目说明：服务端采用POLL形式监控监听和各客户端的连接，分配较大的用户数据数组实现随机访问提高时间利用率；客户端采用管道通信实现与终端的连接建立，也通过POLL实现端口监管。
## 二、信号集处理——基于C++ EPOLL和信号集的事件处理
&emsp;&emsp;1）运用socketpair建立全双工管道，通过EPOLL下的ET模式实现信号的快速接入和管道发送，主循环通过EPOLL实现管道的接受和监听以及发送消息；\
&emsp;&emsp;2）文献来源：《Linux高性能服务器编程》——游双著；通过C++进行了程序改写。
## 三、基于多进程的共享内存
&emsp;&emsp;1）运用多进程实现每个子进程专门管理当前的socket链接，主线程负责监听以及管道；\
&emsp;&emsp;2）文献来源：《Linux高性能服务器编程》——游双著。
