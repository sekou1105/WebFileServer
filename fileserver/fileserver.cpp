#include "fileserver.h"

int WebServer::m_epollfd = -1;         //I/O复用的epoll描述符
bool WebServer::is_stop = false;       //是否停止服务器标志
int WebServer::eventHandlerPipe[2] = {-1, -1};   // 用于统一事件源传递信号的管道

WebServer::WebServer() {

}

WebServer::~WebServer() {

}

void WebServer::init() {
}

int WebServer::create_listenfd(int port, const char *ip) {

    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd < 0) {
        LOG_ERROR("Create socket failed");
        return -1;
    }

    memset(&m_server_addr, 0, sizeof(m_server_addr));
    m_server_addr.sin_family = AF_INET;
    m_server_addr.sin_port = htons(port);
    if (ip != nullptr) {
        inet_pton(AF_INET, ip, &m_server_addr.sin_addr);
    } else {
        m_server_addr.sin_addr.s_addr = INADDR_ANY;
    }

    //设置地址可重用
    int reuse_addr = 1;
    if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) < 0) {
        LOG_ERROR("Set socket reuse options failed");
        close(m_listenfd);
        return -1;
    }

    if (bind(m_listenfd, (struct sockaddr *)&m_server_addr, sizeof(m_server_addr)) < 0) {
        LOG_ERROR("Bind socket failed");
        close(m_listenfd);
        return -1;
    }

    if (listen(m_listenfd, 5) < 0) {
        LOG_ERROR("Listen socket failed");
        close(m_listenfd);
        return -1;
    }

    LOG_INFO("Server listening on %s:%d", ip ? ip : "<ANY>", port);
    return 0;
}

//创建epoll监听套接字
int WebServer::create_epoll() {
    m_epollfd = epoll_create1(0);
    if (m_epollfd < 0) {
        LOG_ERROR("Create epollfd failed");
        return -1;
    }

    return 0;
}

//向epoll中添加监听Listen套接字
int WebServer::epoll_add_listenfd() {
    //Listenfd设置为 边缘触发、非阻塞模式
    set_non_blocking(m_listenfd);

    // 因为需要将连接客户端的任务交给子线程处理，所以设置为边沿触发，避免子线程还没有接受连接时事件一直产生
    int ret = addWaitFd(m_epollfd, m_listenfd, true, false);
    if(ret != 0) {
        LOG_ERROR("Epoll add listen fd failed");
        return -1;
    }
    LOG_INFO("Epoll add listen fd success");

    return 0;
}

//设置监听事件处理的管道
int WebServer::epoll_add_event_pipe() {
    int ret = socketpair(AF_UNIX, SOCK_STREAM, IPPROTO_TCP, eventHandlerPipe);
    if (ret < 0) {
        LOG_ERROR("Create socket pair failed");
        return -1;
    }

    ret = set_non_blocking(eventHandlerPipe[0]);
    if (ret != 0) {
        LOG_ERROR("Set non-blocking event pipe[0] failed");
        return -1;
    }

    ret = set_non_blocking(eventHandlerPipe[1]);
    if (ret != 0) {
        LOG_ERROR("Set non-blocking event pipe[1] failed");
        return -1;
    }

    ret = addWaitFd(m_epollfd, eventHandlerPipe[0]);
    if (ret != 0) {
        LOG_ERROR("Epoll add event pipe[0] failed");
        return -1;
    }

    LOG_INFO("Epoll add event pipe[0] success");
    return 0;
}

//设置term和alarm信号的处理
int WebServer::add_handle_sig(int signo) {
    int ret = 0;

    if(signo == -1) {
        LOG_INFO("信号处理，暂略");
    }


    return 0;
}

//信号处理函数
void WebServer::handle_signal(int signo) {
    //if (signo == SIGINT || signo == SIGTERM) { is_stop = true; return; }
    if((signo & SIGINT) | (signo & SIGTERM)){
        is_stop = true;
        return;
    }
    int saveErrno = errno;
    int msg = signo;
    int ret = send(eventHandlerPipe[1], &msg, 1, 0);
    if(ret != 0){
        LOG_ERROR("Send signal to pipe failed");
    }
    errno = saveErrno;
    return;
}

//主线程负责监听所有事件
int WebServer::wait_epoll() {
    // 标识服务器是否暂停
    is_stop = false;

    while(!is_stop) {
        int resNum = epoll_wait(m_epollfd, resEvents, MAX_RESEVENT_SIZE, -1);
        //如果出错，直接退出（因为事件发生导致返回-1时，errno会置ENITR，需要在事件处理函数中保留errno）
        if(resNum < 0 && errno != EINTR) {
            LOG_ERROR("Epoll wait failed");
            return -1;
        }
        //ET模式触发
        //et(resEvents, resNum, m_epollfd, m_listenfd);
        //LT模式触发
        lt(resEvents, resNum, m_epollfd, m_listenfd);

    }
    return 0;
}
