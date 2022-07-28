/* 基于POLL的服务端多客户端监听 ，基于《 Linux高性能服务器编程 》中的聊天室项目进行学习和修改 */
#define _GNU_SOURCE 1//表示支持TCP关闭连接事件
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<poll.h>
#include<iostream>
#define USER_LIMIT 5//最大用户数量
#define BUFFER_SIZE 64//读缓冲区大小
#define FD_LIMIT 65535//文件描述符数量限制
using namespace std;
struct client_data{//客户数据：客户端socket地址、待写到客户端的数据的位置、从客户端读入的数据。
    sockaddr_in address;
    char* write_buf;
    char buf[BUFFER_SIZE];
};
int setnonblocking(int fd){//设置文件描述符为非阻塞
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}
int main(int argc,char* argv[]){
    if(argc<=2){
        cout<<"ip or port error"<<endl;
        return 1;
    }
    const char* ip=argv[1];
    int port=atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in address;//服务器地址
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port=htons(port);
    int listenfd=socket(PF_INET,SOCK_STREAM,0);
    assert(listenfd>=0);
    ret=bind(listenfd,(struct sockaddr*)&address,sizeof(address));//bind listenfd and socket
    assert(ret!=-1);
    ret=listen(listenfd,USER_LIMIT);
    assert(listenfd!=-1);
    client_data* users=new client_data[FD_LIMIT];//数据用户组
    pollfd fds[USER_LIMIT + 1];//pollfd结构体
    int user_counttotal = 0;
    for(int i=0;i<=USER_LIMIT;i++){//初始化poll事件
        fds[i].fd = -1;
        fds[i].events=0;
    }
    fds[0].fd=listenfd;
    fds[0].events=POLLIN|POLLERR;//监听端口注册可读和错误事件
    fds[0].revents=0;
    while(1){
        ret=poll(fds,user_counttotal + 1,-1);//无限期等待注册事件就绪
        if(ret<0){
            cout<<"poll error"<<endl;
            break;
        }
        for(int i=0;i<user_counttotal + 1;i++){
            if((fds[i].fd==listenfd)&&(fds[i].revents&POLLIN)){//有新用户请求连接
                struct sockaddr_in client_address;
                socklen_t client_addrlength=sizeof(client_address);
                int connfd=accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);
                if(connfd<0){
                    cout<<"error no is:"<<strerror(errno)<<endl;
                    continue;
                }
                if(user_counttotal>=USER_LIMIT){//超出最大用户数目
                    const char* info="too many users\n";
                    cout<<info<<endl;
                    send(connfd,info,strlen(info),0);
                    close(connfd);
                    continue;
                }
                user_counttotal++;
                users[connfd].address=client_address;//新用户插入
                setnonblocking(connfd);
                fds[user_counttotal].fd=connfd;
                fds[user_counttotal].events=POLLIN|POLLRDHUP|POLLERR;
                fds[user_counttotal].revents=0;
                cout<<"comes a new user id:"<<user_counttotal<<endl;
            }
            else if(fds[i].revents&POLLERR){//连接出错
                cout<<"poll error in:"<<fds[i].fd<<endl;
                char errors[100];
                memset(errors,'\0',100);
                socklen_t length=sizeof(errors);
                if(getsockopt(fds[i].fd,SOL_SOCKET,SO_ERROR,&errors,&length)<0){
                    cout<<"get socket option error"<<endl;
                }
                continue;
            }
            else if(fds[i].revents&POLLRDHUP){//断开连接
                users[fds[i].fd]=users[fds[user_counttotal].fd];
                close(fds[i].fd);
                fds[i]=fds[user_counttotal];
                i--;
                user_counttotal--;
                cout<<"a client left"<<endl;
            }
            else if(fds[i].revents&POLLIN){//客户端向服务端发送消息
                int connfd=fds[i].fd;
                memset(users[connfd].buf,'\0',BUFFER_SIZE-1);
                ret=recv(connfd,users[connfd].buf,BUFFER_SIZE-1,0);
                cout<<"data comes"<<users[connfd].buf<<"  from client:"<<connfd<<" lenth is:"<<ret<<endl;
                if(ret<0){
                    if(errno!=EAGAIN){
                        close(connfd);//否则断开用户连接
                        users[fds[i].fd]=users[fds[user_counttotal].fd];
                        fds[i]=fds[user_counttotal];
                        i--;
                        user_counttotal--;
                    }
                }
                else if(ret==0){ }
                else{
                    for(int j=1;j<=user_counttotal;j++){//其他用户发送
                        if(fds[j].fd==connfd){
                            continue;
                        }
                        fds[j].events&=~POLLIN;//此处与源代码不一致，如果按照源代码，事件处理的就会变得很多
                        fds[j].events|=POLLOUT;
                        users[fds[j].fd].write_buf=users[connfd].buf;
                    }
                }
            }
            else if(fds[i].revents&POLLOUT){//发送后改回读数据判断
                int connfd=fds[i].fd;
                if(!users[connfd].write_buf){
                    continue;
                }
                ret=send(connfd,users[connfd].write_buf,strlen(users[connfd].write_buf),0);
                users[connfd].write_buf=NULL;//恢复当前用户数据
                fds[i].events&=~POLLOUT;//此处与源代码不一致，如果按照源代码，事件处理的就会变得很多
                fds[i].events|=POLLIN;//###1###
            }
        }
    }
    delete[] users;
    close(listenfd);
    return 0;
}
