#ifndef __FILESERVER__
#define __FILESERVER__

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../event/myevent.h"

#define MAX_RESEVENT_SIZE 1024   // 事件的最大个数

class WebServer
{
public:
    WebServer();

    ~WebServer();

    void init();

    //创建监听套接字，等待客户端连接，并开启监听
    int create_listenfd(int port, const char *ip = nullptr);

    //创建epoll监听套接字
    int create_epoll();

    //向epoll中添加监听Listen套接字
    int epoll_add_listenfd();

    //设置监听事件处理的管道
    int epoll_add_event_pipe();

    //设置term和alarm信号的处理
    int add_handle_sig(int signo = -1);

    //信号处理函数
    void handle_signal(int signo);

    //主线程负责监听所有事件
    int wait_epoll();

    //创建线程池
    int createThreadPool(int threadNum = 8);

private:
    int m_listenfd;             //服务器端的套接字
    struct sockaddr_in m_server_addr;  //服务器地址
    static int m_epollfd;         //I/O复用的epoll描述符
    static bool is_stop;       //是否停止服务器标志

    static int eventHandlerPipe[2];             // 用于统一事件源传递信号的管道
    epoll_event resEvents[MAX_RESEVENT_SIZE];   // 保存 epoll_wait 结果的数组
    ThreadPool *m_thread_pool;

};

#endif // __FILESERVER__
