#include "utils.h"

// 以 "09:50:19.0619 2022-09-26 [logType]: " 格式返回当前的时间和输出类型，logType 指定输出的类型：
// init  : 表示服务器的初始化过程
// error : 表示服务器运行中的出错消息
// info  : 表示程序的运行信息
std::string outHead(const std::string logType){
    // 获取并输出时间
    auto now = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    auto time_tm = localtime(&tt);
    
    struct timeval time_usec;
    gettimeofday(&time_usec, NULL);

    char strTime[30] = { 0 };
    //16:31:15.695681 2025-05-10
    sprintf(strTime, "%02d:%02d:%02d.%05ld %d-%02d-%02d",
            time_tm->tm_hour, time_tm->tm_min, time_tm->tm_sec, time_usec.tv_usec,
            time_tm->tm_year + 1900, time_tm->tm_mon + 1, time_tm->tm_mday);

    std::string outStr;
    // 添加时间部分
    outStr += strTime;
    // 根据传入的参数指定输出的类型
    if(logType == "init"){
        outStr += " [init]: ";
    }else if(logType == "error"){
        outStr += " [erro]: ";
    }else{
        outStr += " [info]: ";
    }
    //得到16:31:15.695681 2025-05-10 [info]:
    return outStr;
}

//设置非阻塞
int set_non_blocking(int fd) {
    int old_flags = fcntl(fd, F_GETFL, 0);
    if (old_flags == -1) {
        return -1;
    }
    return fcntl(fd, F_SETFL, old_flags | O_NONBLOCK);
}

// 向 epollfd 添加文件描述符，并指定监听事件。edgeTrigger：边缘触发，isOneshot：EPOLLONESHOT
int addWaitFd(int epollFd, int newFd, bool edgeTrigger, bool isOneshot) {
    //1、创建epoll事件对象
    struct epoll_event event;
    //2、初始化事件类型、设置事件对象的描述符
    event.events = EPOLLIN;
    event.data.fd = newFd;
    //3、是否设置ET、ONESHOT模式
    if (edgeTrigger) {
        event.events |= EPOLLET;
    }
    if (isOneshot) {
        event.events |= EPOLLONESHOT;
    }

    //4、将事件对象添加到epoll中
    int ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, newFd, &event);
    if (ret < 0) {
        LOG_ERROR("Epoll add fd failed");
    }

    return ret;
}

// 删除正在监听的文件描述符
int deleteWaitFd(int epollFd, int deleteFd){
    int ret = epoll_ctl(epollFd, EPOLL_CTL_DEL, deleteFd, nullptr);
    if(ret != 0){
        std::cout << outHead("error") << "删除监听的文件描述符失败" << std::endl;
        return -1;
    }
    return 0;
}

// 修改正在监听文件描述符的事件。edgeTrigger:是否为边沿触发，resetOneshot:是否设置 EPOLLONESHOT，addEpollout:是否监听输出事件
int modifyWaitFd(int epollFd, int modFd, bool edgeTrigger, bool resetOneshot, bool addEpollout){
    epoll_event event;
    event.data.fd = modFd;

    event.events = EPOLLIN;

    if(edgeTrigger){
        event.events |= EPOLLET;
    }
    if(resetOneshot){
        event.events |= EPOLLONESHOT;
    }
    if(addEpollout){
        event.events |= EPOLLOUT;
    }

    int ret = epoll_ctl(epollFd, EPOLL_CTL_MOD, modFd, &event);
    if(ret != 0){
        std::cout << outHead("error") << "修改文件描述符失败" << std::endl;
        return -1;
    }
    return 0;
}

//ET模式的工作流程
void et(struct epoll_event* events, int number, int epollfd, int listenfd) {
    char buf[BUFF_SIZE];
    for (int i = 0; i < number; i++) {
        int sockfd = events[i].data.fd;
        if (sockfd == listenfd) {
            struct sockaddr_in client_addr;
            socklen_t client_addrlen = sizeof(client_addr);

            int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addrlen);

            addWaitFd(epollfd, connfd, true);   //ET模式

        } else if (events[i].events & EPOLLIN) {
            //这段代码不会重复触发，只有当socket上有数据可读时才会触发
            //ET模式下，EPOLLIN事件只会触发一次，直到数据被读完
            //因此需要循环读取数据，直到没有数据可读为止
            LOG_INFO("event ET trigger once");
            while (1) {
                memset(buf, 0, sizeof(buf));
                int ret = recv(sockfd, buf, sizeof(buf)-1, 0);
                if (ret < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        LOG_INFO("read later");
                        break; //数据读完了
                    }
                    close(sockfd);
                    break;
                } else if (ret == 0) {
                    close(sockfd);
                    break; //对端关闭连接
                }
                LOG_INFO("get %d bytes of content: %s", ret, buf);
            }
        } else {
            LOG_INFO("something else happened");
        }
    }

}

//lt模式工作流程
void lt(struct epoll_event *events, int number, int epollfd, int listenfd) {
    char buf[BUFF_SIZE];
    for (int i = 0; i < number; i++) {
        int sockfd = events[i].data.fd;
        if (sockfd == listenfd) {
            struct sockaddr_in client_addr;
            socklen_t client_addrlen = sizeof(client_addr);

            int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addrlen);
            addWaitFd(epollfd, connfd, false);   //LT模式

        } else if (events[i].events & EPOLLIN) {
            //只要socket上有数据可读，就会触发EPOLLIN事件
            //LT模式下，EPOLLIN事件会一直触发，直到数据被读完
            //因此需要循环读取数据，直到没有数据可读为止
            LOG_INFO("lt event trigger once");
            memset(buf, 0, sizeof(buf));
            int ret = recv(sockfd, buf, sizeof(buf)-1, 0);
            if (ret <= 0) {
                close(sockfd);
                continue;
            }
            LOG_INFO("get %d bytes of content: %s", ret, buf);
        } else {
            LOG_INFO("something else happened");
        }
    }
}

