#ifndef __UTILS__
#define __UTILS__
#include <iostream>
#include <string.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <sys/time.h>
#include <chrono>

#include "../log/log.h"

#define BUFF_SIZE 10

std::string outHead(const std::string logType);

//设置文件描述符为非阻塞
int set_non_blocking(int fd);

// 向 epollfd 添加文件描述符，并指定监听事件。edgeTrigger：边缘触发，isOneshot：EPOLLONESHOT
int addWaitFd(int epollFd, int newFd, bool edgeTrigger = false, bool isOneshot = false);

// 修改正在监听文件描述符的事件。edgeTrigger:是否为边沿触发，resetOneshot:是否设置 EPOLLONESHOT，addEpollout:是否监听输出事件
int modifyWaitFd(int epollFd, int modFd, bool edgeTrigger = false, bool resetOneshot = false, bool addEpollout = false );

// 删除正在监听的文件描述符
int deleteWaitFd(int epollFd, int deleteFd);

//ET模式的工作流程
void et(struct epoll_event* events, int number, int epollfd, int listenfd);

//lt模式工作流程
void lt(struct epoll_event *events, int number, int epollfd, int listenfd);

#endif// __UTILS__