#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <unordered_map>

/*
 * Request报文结构：
 * 
 * GET /api/data HTTP/1.1          // 起始行：方法+路径+协议版本
 * Host: api.example.com           // 必需消息首部（HTTP/1.1要求）
 * User-Agent: Mozilla/5.0
 * Accept: application/json
 * 
 * { "query": "test" }             // 请求体（GET通常无请求体）
 */

/*
 * Response报文结构：
 * 
 * HTTP/1.1 200 OK                  // 状态行：版本+状态码+描述
 * Content-Type: application/json    // 响应消息首部
 * Content-Length: 20                // 响应消息首部
 * 
 * { "data": "test" }                // 响应体
 */

// Request 或 Response 中数据的处理状态
enum MSGSTATUS{
    HANDLE_INIT,      // 正在接收/发送头部数据（请求行、请求头）
    HANDLE_HEAD,      // 正在接收/发送消息首部
    HANDLE_BODY,      // 正在接收/发送消息体
    HADNLE_COMPLETE,  // 所有数据都已经处理完成
    HANDLE_ERROR,     // 处理过程中发生错误
};

// 表示消息体的类型
enum MSGBODYTYPE{
    FILE_TYPE,      // 消息体是文件
    HTML_TYPE,      // 消息体是 HTML 页面
    JSON_TYPE,      // 消息体是 JSON 数据
    EMPTY_TYPE,     // 消息体为空
};

// 当接收文件时，消息体会分不同的部分，用该类型表示文件消息体已经处理到哪个部分
enum FILEMSGBODYSTATUS{
    FILE_BEGIN_FLAG,   // 正在获取并处理表示文件开始的标志行
    FILE_HEAD,         // 正在获取并处理文件属性部分
    FILE_CONTENT,      // 正在获取并处理文件内容的部分
    FILE_COMPLATE      // 文件已经处理完成
};

class Message{
public:
    //默认构造函数，成员初始化列表。
    Message();

public:
    // 请求消息和响应消息都需要使用的一些成员
    // 记录消息的接收状态，表示整个请求报文收到了多少/发送了多少
    MSGSTATUS m_status;
    // 保存消息首部，存储的是键值对（key-value pairs）
    // 首部字段 map（如 Content-Type, Content-Length）
    std::unordered_map<std::string, std::string> m_msgHeader;  
};

//请求消息
//继承自 Message，代表从浏览器接收到的 HTTP 请求：
// 继承 Message，对请求行的修改和获取，保存收到的首部选项
class Request : public Message{
public:
    Request();

    void setRequestLine(const std::string &requestLine);

    void addHeaderOpt(const std::string &headLine);

public:
    std::string m_recvMsg;              // 收到但是还未处理的数据

    std::string m_requestMethod;        // 请求消息的请求方法
    std::string m_requestResource;      // 请求的资源
    std::string m_httpVersion;          // 请求的HTTP版本

    long long m_contentLength = 0;      // 记录消息体的长度
    long long m_msgBodyRecvLen;         // 已经接收的消息体长度

    std::string m_recvFileName;
    FILEMSGBODYSTATUS fileMsgStatus;       // 记录表示文件的消息体已经处理到哪些部分
private:

};

// 响应消息
// 继承 Message，对于状态行修改和获取，设置要发送的首部选项
class Response : public Message{
public:
    Response() : Message(){

    }

public:
    // 保存状态行相关数据
    std::string m_responseHttpVersion = "HTTP/1.1";
    std::string m_responseStatusCode;  // 如 200、404
    std::string m_responseStatusDes;   // 如 OK、Not Found

    // 以下成员主要用于在发送响应消息时暂存相关的数据

    MSGBODYTYPE m_bodyType;                                 // 消息的类型
    std::string m_bodyFileName;                             // 要发送数据的路径


    std::string m_beforeBodyMsg;                            // 消息体之前的所有数据
    int m_beforeBodyMsgLen;                                 // 消息体之前的所有数据的长度

    std::string m_msgBody;                                  // 在字符串中保存 HTML / JSON 或其它类型的消息体
    unsigned long m_msgBodyLen;                             // 消息体的长度

    int m_fileMsgFd;                                        // 文件类型的消息体保存文件描述符

    unsigned long m_curStatusHasSendLen;                    // 记录在当前状态下，这些数据已经发送的长度
private:
    
};

#endif