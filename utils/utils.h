#ifndef __UTILS__
#define __UTILS__
#include <iostream>
#include <string.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../log/log.h"

#define BUFF_SIZE 10

extern int m_close_log;

//设置文件描述符为非阻塞
int set_non_blocking(int fd);

// 向 epollfd 添加文件描述符，并指定监听事件。edgeTrigger：边缘触发，isOneshot：EPOLLONESHOT
int addWaitFd(int epollFd, int newFd, bool edgeTrigger = false, bool isOneshot = false);

//ET模式的工作流程
void et(struct epoll_event* events, int number, int epollfd, int listenfd);

//lt模式工作流程
void lt(struct epoll_event *events, int number, int epollfd, int listenfd);

#endif// __UTILS__