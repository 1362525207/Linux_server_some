# Linux_shell_chat
基于C++和POLL的服务端和客户端
## 一、学习参考
&emsp;&emsp;1）文献来源：《Linux高性能服务器编程》——游双著；\
&emsp;&emsp;2）参考学习：版权声明：本文为CSDN博主「liuxuejiang158」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。原文链接：https://blog.csdn.net/liuxuejiang158blog/article/details/12503269
## 二、使用说明
&emsp;&emsp;1）终端输入./server ip port即可运行服务端；终端输入./client ip port即可运行服务端；
&emsp;&emsp;2）项目说明：服务端采用POLL形式监控监听和各客户端的连接，分配较大的用户数据数组实现随机访问提高时间利用率；客户端采用管道通信实现与终端的连接建立，也通过POLL实现端口监管。
