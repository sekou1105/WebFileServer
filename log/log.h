#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "../threadpool/threadpool.h"

/*
    单例模式的日志类，构造和析构函数权限private。
    只有类内部的成员方法可以创建对象；
    避免重复初始化；
    全局访问；
    线程安全；
*/
class LOG 
{
public:
    static LOG *get_instance();

    static void *flush_log_thread(void *args);

    //初始化日志，文件名称、关闭标志、日志缓冲区大小、行数、最长队列
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush(void);
     
private:
    LOG();

    virtual ~LOG();
    
    void *async_write_log();

private:
    FILE *m_fp;             //打开日志的文件指针
    char dir_name[128];     //日志文件目录
    char log_name[128];     //日志文件名
    int m_split_lines;      //日志最大行数
    int m_log_buf_size;     //日志缓冲区大小
    long long m_count;      //日志行数
    int m_today;            //日志天数
    char *m_buf;            //日志缓冲区
    //block_queue<std::string> m_log_queue;
    bool m_is_async;
    Locker m_mutex;
    int m_close_log;        //关闭日志标志


};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) { LOG::get_instance()->write_log(0, "[%s:%d][%s] " format, __FILE__, __LINE__, __func__, ##__VA_ARGS__); LOG::get_instance()->flush(); }
#define LOG_INFO(format, ...)  if(0 == m_close_log) { LOG::get_instance()->write_log(1, "[%s:%d][%s] " format, __FILE__, __LINE__, __func__, ##__VA_ARGS__); LOG::get_instance()->flush(); }
#define LOG_WARN(format, ...)  if(0 == m_close_log) { LOG::get_instance()->write_log(2, "[%s:%d][%s] " format, __FILE__, __LINE__, __func__, ##__VA_ARGS__); LOG::get_instance()->flush(); }
#define LOG_ERROR(format, ...) if(0 == m_close_log) { LOG::get_instance()->write_log(3, "[%s:%d][%s] " format, __FILE__, __LINE__, __func__, ##__VA_ARGS__); LOG::get_instance()->flush(); }

#endif // __LOG_H__