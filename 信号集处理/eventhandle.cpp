#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <iostream>
#define MAX_EVENT_NUMBER 1024
using namespace std;
static int pipefd[2];//pipe
void sig_handler( int sig )
{
    int save_errno =errno;//errno对信号等事件的错误，相当于一种全局变量
    int msg = sig;
    send( pipefd[1], ( char* )&msg, 1, 0 );
    errno = save_errno;
};
class fd_and_sig{
public:
	int setnonblocking( int fd )
	{
    		int old_option = fcntl( fd, F_GETFL );//获取之前的控制符号
    		int new_option = old_option | O_NONBLOCK;
    		fcntl( fd, F_SETFL, new_option );//将当前文件描述符改为非阻塞
    		return old_option;
	};
public:
	void addfd( int epollfd, int fd )
	{
	    epoll_event event;
	    event.data.fd = fd;
	    event.events = EPOLLIN | EPOLLET;//可读事件和ET模式，也就是当检测到事情后必须立马处理
	    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );//表示往注册表中注册事件
	    setnonblocking( fd );
	};
	void addsig( int sig )
	{
	    struct sigaction sa;//对sigaction的一种调用
	    memset( &sa, '\0', sizeof( sa ) );
	    sa.sa_handler = sig_handler;
	    sa.sa_flags |= SA_RESTART;
	    sigfillset( &sa.sa_mask );//信号集设置所有信号
	    assert( sigaction( sig, &sa, NULL ) != -1 );
	}
};

int main( int argc, char* argv[] )
{
    if( argc <= 2 )
    {
        cout<<"argc!=2"<<endl;
        return 1;
    }
     fd_and_sig fdsig;
    const char* ip = argv[1];
    int port = atoi( argv[2] );

    int ret = 0;
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );

    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( listenfd >= 0 );

    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    if( ret == -1 )
    {
        printf( "errno is %d\n", errno );
        return 1;
    }

    ret = listen( listenfd, 5 );
    assert( ret != -1 );

    epoll_event events[ MAX_EVENT_NUMBER ];
    int epollfd = epoll_create( 5 );
    assert( epollfd != -1 );
    fdsig.addfd( epollfd, listenfd );

    ret = socketpair( PF_UNIX, SOCK_STREAM, 0, pipefd );
    assert( ret != -1 );
    fdsig.setnonblocking( pipefd[1] );
    fdsig.addfd( epollfd, pipefd[0] );

    fdsig.addsig( SIGHUP );
    fdsig.addsig( SIGCHLD );
    fdsig.addsig( SIGTERM );
    fdsig.addsig( SIGINT );
    bool stop_server = false;

    while( !stop_server )
    {
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );
        if ( ( number < 0 ) && ( errno != EINTR ) )
        {
            cout<<"epoll error"<<endl;
            break;
        }
        for ( int i = 0; i < number; i++ )
        {
            int sockfd = events[i].data.fd;
            if( sockfd == listenfd )
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
                fdsig.addfd( epollfd, connfd );
            }
            else if( ( sockfd == pipefd[0] ) && ( events[i].events & EPOLLIN ) )
            {
                int sig;
                char signals[1024];
                ret = recv( pipefd[0], signals, sizeof( signals ), 0 );
                if( ret == -1 || ret==0 )
                {
                    continue;
                }
                else
                {
                    for( int i = 0; i < ret; ++i )
                    {
                        
                        switch( signals[i] )
                        {
                            case SIGCHLD: cout<<"sigchld"<<endl;
                            case SIGHUP:
                            {
					cout<<"sighup"<<endl;
                                continue;
                            }
                            case SIGTERM: cout<<"sigterm"<<endl;
                            case SIGINT:
                            {
                                stop_server = true;
                            }
                        }
                    }
                }
            }
            else
            {
            }
        }
    }

    cout<< "close fds" <<endl;
    close( listenfd );
    close( pipefd[1] );
    close( pipefd[0] );
    return 0;
}

