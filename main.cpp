#include "./fileserver/fileserver.h"

// 全局变量
int m_close_log = 0;  // 0=开启日志, 1=关闭日志

int main(int argc, char *argv[])
{   
    WebServer server;

    server.init(0, 0);

    server.log_write();

    int port = 8888;
    int ret = server.create_listenfd(port);
    if(ret != 0){
        LOG_ERROR("create listenfd failed.");
        return -2;
    }

    ret = server.create_epoll();
    if(ret != 0){
        LOG_ERROR("create epollfd failed.");
        return -3;
    }

    ret = server.epoll_add_listenfd();
    if(ret != 0) {
        LOG_ERROR("epoll add listenfd failed.");
        return -4;
    }

    ret = server.wait_epoll();
    if(ret != 0) {
        LOG_ERROR("epoll listen failed.");
        return -5;
    }
    
    return 0;
}