#include "./fileserver/fileserver.h"

int main(int argc, char *argv[])
{   
    WebServer server;

    // 初始化日志：默认同步写入，可通过 SetLogAsync/SetLogClose 运行时修改
    InitLog("./ServerLog", 0, 2000, 800000, 0);
    LOG_INFO("This is an info log, init success");

    server.init();

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