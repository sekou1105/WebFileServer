# WebFileServer
File server

## 参考项目
    https://github.com/shangguanyongshi/WebFileServer
    https://github.com/qinguoyi/TinyWebServer

## 测试流程
1、上传文件
curl -i -X POST http://127.0.0.1:8888/upload \
  -F "file=@/path/to/local/file.bin"

2、下载文件
curl -L "http://127.0.0.1:8888/download/测试文件.txt" -o test.txt

3、删除文件
curl -i "http://127.0.0.1:8888/delete/filename.bin"

4、获取文件列表
