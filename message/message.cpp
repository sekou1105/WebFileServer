#include "message.h"

Message::Message(): m_status(HANDLE_INIT) { }

Request::Request(): Message() { }

//该函数常用于解析HTTP请求的第一行（请求行），其标准格式为：
// <method> <request-target> <HTTP-version>
// 设置与返回请求行相关字段
    /**
    * @brief 设置请求行
    *
    * 从传入的字符串中提取请求方法、请求资源和HTTP版本，并保存到类中相应的成员变量中。
    * 比如 GET /index.html HTTP/1.1
    * @param requestLine 请求行字符串
    */
void Request::setRequestLine(const std::string &requestLine) {
    std::istringstream lineStream(requestLine);
    // 获取请求方法
    lineStream >> m_requestMethod;   // GET/POST等
    // 获取请求资源
    // lineStream >> rquestResourse;
    lineStream >> m_requestResource;  // 请求的资源路径

    // 获取http版本
    lineStream >> m_httpVersion;  // 协议版本

    return;
}

// 对于Request 报文，根据传入的一行首部字符串，向首部保存选项
    /**
     * @brief 添加头部选项
     *
     * 该函数将传入的头部选项解析并存入 msgHeader 中。
     * 比如Content-Type: multipart/form-data; boundary=xxx
     * @param headLine 要解析的头部选项字符串
     */
void Request::addHeaderOpt(const std::string &headLine){
    static std::istringstream lineStream;
        lineStream.str(headLine);    // 以 istringstream 的方式处理头部选项

        std::string key, value;      // 保存键和值的临时量

        lineStream >> key;           // 获取 key
        key.pop_back();              // 删除键中的冒号 
        lineStream.get();            // 删除冒号后的空格

        // 读取空格之后所有的数据，遇到 \n 停止，所以 value 中还包含一个 \r
        getline(lineStream, value);
        value.pop_back();            // 删除其中的 \r
        
        if(key == "Content-Length"){
            // 保存消息体的长度
            m_contentLength = std::stoll(value);

        }else if(key == "Content-Type"){
            // 分离消息体类型。消息体类型可能是复杂的消息体，类似 Content-Type: multipart/form-data; boundary=---------------------------24436669372671144761803083960
            
            // 先找出值中分号的位置
            std::string::size_type semIndex = value.find(';');
            // 根据分号查找的结果，保存类型的结果
            if(semIndex != std::string::npos){
                m_msgHeader[key] = value.substr(0, semIndex);
                std::string::size_type eqIndex = value.find('=', semIndex);
                key = value.substr(semIndex + 2, eqIndex - semIndex - 2);
                m_msgHeader[key] = value.substr(eqIndex + 1);
            }else{
                m_msgHeader[key] = value;
            }
            
        }else{
            m_msgHeader[key] = value;
        }
}
