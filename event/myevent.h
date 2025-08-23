#ifndef __MYEVENT_H__
#define __MYEVENT_H__

#include <dirent.h>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdio>

#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include "../message/message.h"
#include "../utils/utils.h"

// 所有事件的基类
class EventBase{
public:
    EventBase(){

    }
    virtual ~EventBase(){

    }
protected:
    // 保存文件描述符对应的请求的状态，因为一个连接上的数据可能非阻塞一次读取不完，所以保存到这里，当该连接上有新数据时，可以继续读取并处理
    static std::unordered_map<int, Request> requestStatus;

    // 保存文件描述符对应的发送数据的状态，一次proces中非阻塞的写数据可能无法将数据全部传过去，所以保存当前数据发送的状态，可以继续传递数据
    static std::unordered_map<int, Response> responseStatus;
    //所以即使一次 read() 或 send() 没完成，也能“断点续传”。

public:
    // 不同类型事件中重写该函数，执行不同的处理方法
    virtual void process(){
        
    }
};

// 用于接受客户端连接的事件
class AcceptConn : public EventBase{
public:
    AcceptConn(int listenFd, int epollFd): m_listenFd(listenFd), m_epollFd(epollFd){ };
    virtual ~AcceptConn(){ };

public:
    virtual void process() override;

private:
    int m_listenFd;              // 保存监听套接字 
    int m_epollFd;               // 接收连接后加入的 epoll
    int m_accetpFd;                // 保存接受的连接

    sockaddr_in m_clientAddr;      // 客户端地址
    socklen_t m_clientAddrLen;     // 客户端地址长度
};

// 用于处理信号的事件，暂时没有处理信号事件需要，没有具体实现
class HandleSig : public EventBase{
public:
    HandleSig(int epollFd) : EventBase(){ };
    virtual ~HandleSig(){ };
public:
    virtual void process() override{ };

private:
};

// 处理客户端发送的请求
class HandleRecv : public EventBase{
public:
    HandleRecv(int clientFd, int epollFd) : m_clientFd(clientFd), m_epollFd(epollFd){ };
    virtual ~HandleRecv(){ };
public:
    virtual void process() override;

private:
    int m_clientFd;   // 客户端套接字，从该客户端读取数据
    int m_epollFd;    // epoll 文件描述符，在需要重置事件或关闭连接时使用


};

// 处理向客户端发送数据
class HandleSend : public EventBase{
public:
    HandleSend(int clientFd, int epollFd) : m_clientFd(clientFd), m_epollFd(epollFd){ };
    virtual ~HandleSend(){ };

public:
    virtual void process() override;
    
    // 用于构建状态行，参数分别表示状态行的三个部分
    std::string getStatusLine(const std::string &httpVersion, const std::string &statusCode, const std::string &statusDes);

    // 以下两个函数用来构建文件列表的JSON返回内容
    void getFileListJson(std::string &fileListJson, const char *filedir = "filedir");

    void getFileVec(const std::string dirName, std::vector<std::string> &resVec);

    // 构建头部字段：
    // contentLength        : 指定消息体的长度
    // contentType          : 指定消息体的类型
    // redirectLoction = "" : 如果是重定向报文，可以指定重定向的地址。空字符串表示不添加该首部。
    // contentRange = ""    : 如果是下载文件的响应报文，指定当前发送的文件范围。空字符串表示不添加该首部。
    std::string getMessageHeader(const std::string contentLength, const std::string contentType, const std::string redirectLoction = "", const std::string contentRange = "");


private:
    int m_clientFd;   // 客户端套接字，向该客户端写数据
    int m_epollFd;    // epoll 文件描述符，在需要重置事件或关闭连接时使用
};
#endif