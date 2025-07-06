/*
 * @Author       : GameServer
 * @Date         : 2024-01-01
 * @Description  : 异步日志系统
 */ 

#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>           
#include <assert.h>
#include <sys/stat.h>         
#include <fstream>
#include <memory>
#include "blockqueue.h"
#include "buffer.h"

class Log {
public:
    void init(int level, const char* path = "./log", 
                const char* suffix =".log",
                int maxQueueCapacity = 1024);

    static Log* Instance();
    static void FlushLogThread();

    void write(int level, const char *format,...);
    void flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen() { return isOpen_; }
    
private:
    Log();
    void AppendLogLevelTitle_(int level);
    virtual ~Log();
    void AsyncWrite_();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path_;
    const char* suffix_;

    int lineCount_;
    int toDay_;

    bool isOpen_;
 
    Buffer buff_;
    int level_;
    bool isAsync_;

    FILE* fp_;
    std::unique_ptr<BlockDeque<std::string>> deque_; 
    std::unique_ptr<std::thread> writeThread_;
    std::mutex mtx_;
};

// 标准C++兼容的日志宏定义
#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, __VA_ARGS__); \
            log->flush();\
        }\
    } while(0)

// 为了处理无参数的情况，定义一个专门的宏
#define LOG_BASE_SIMPLE(level, format) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format); \
            log->flush();\
        }\
    } while(0)

#define LOG_DEBUG(format, ...) LOG_BASE(0, format, __VA_ARGS__)
#define LOG_INFO(format, ...) LOG_BASE(1, format, __VA_ARGS__)
#define LOG_WARN(format, ...) LOG_BASE(2, format, __VA_ARGS__)
#define LOG_ERROR(format, ...) LOG_BASE(3, format, __VA_ARGS__)

// 简单版本（无参数）
#define LOG_DEBUG_SIMPLE(format) LOG_BASE_SIMPLE(0, format)
#define LOG_INFO_SIMPLE(format) LOG_BASE_SIMPLE(1, format)
#define LOG_WARN_SIMPLE(format) LOG_BASE_SIMPLE(2, format)
#define LOG_ERROR_SIMPLE(format) LOG_BASE_SIMPLE(3, format)

#endif //LOG_H 