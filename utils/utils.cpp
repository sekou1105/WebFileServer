#include "utils.h"

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

