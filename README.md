# Linux_shell_chat
基于C++和POLL的服务端和客户端——非文件夹安放文件
## 一、学习参考
&emsp;&emsp;1）文献来源：《Linux高性能服务器编程》——游双著；\
&emsp;&emsp;2）参考学习：https://blog.csdn.net/liuxuejiang158blog/article/details/12503269
## 二、使用说明
&emsp;&emsp;1）终端输入./server ip port即可运行服务端；终端输入./client ip port即可运行客户端；\
&emsp;&emsp;2）项目说明：服务端采用POLL形式监控监听和各客户端的连接，分配较大的用户数据数组实现随机访问提高时间利用率；客户端采用管道通信实现与终端的连接建立，也通过POLL实现端口监管。
## 三、信号集处理——基于EPOLL和信号集的事件处理
&emsp;&emsp;运用socketpair建立全双工管道，通过EPOLL下的ET模式实现信号的快速接入和管道发送，主循环通过EPOLL实现管道的接受和监听以及发送消息；
