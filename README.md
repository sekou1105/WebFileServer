# WebFileServer
文件服务器

## 参考项目
    https://github.com/shangguanyongshi/WebFileServer
    https://github.com/qinguoyi/TinyWebServer

## 功能说明
1、C/C++
2、B/S模型（目前只有服务端，没有前端）
3、Reactor模型：epoll + ET/非阻塞/EPOLLONESHOT + 线程池
4、http有限状态机 + 事件处理
5、同步/异步日志

## 存在问题
1、请求/list接口失败
2、删除重定向逻辑
【放在dev分支修改】

## 后续改进
1、补充前端内容，新增用户注册、登录、鉴权等接口
2、新增路由模块，处理HTTP请求
3、新增数据库连接池
4、完善定时器的功能
5、启动长连接功能
6、新增文件共享等功能，类似于网盘


## 测试流程
1、上传文件
curl -i -X POST http://127.0.0.1:8888/upload \
  -F "file=@/path/to/local/file.bin"

2、下载文件
curl -L "http://127.0.0.1:8888/download/测试文件.txt" -o test.txt

3、删除文件
curl -i "http://127.0.0.1:8888/delete/filename.bin"

4、获取文件列表
curl "http://127.0.0.1:8888/"
