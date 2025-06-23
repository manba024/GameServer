#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace IOMultiplexing {

// IO事件类型
enum class IOEventType {
    Read = 1,      // 可读事件
    Write = 2,     // 可写事件
    Error = 4,     // 错误事件
    HangUp = 8     // 连接挂起
};

// IO事件结构
struct IOEvent {
    int fd;                    // 文件描述符
    uint32_t events;           // 事件类型掩码
    void* userData;            // 用户数据指针
    
    IOEvent(int f = -1, uint32_t e = 0, void* data = nullptr) 
        : fd(f), events(e), userData(data) {}
};

// 事件回调函数类型
using EventCallback = std::function<void(const IOEvent&)>;

// IO复用器抽象基类
class IOMultiplexer {
public:
    virtual ~IOMultiplexer() = default;

    // 添加文件描述符及其关注的事件
    virtual bool addFd(int fd, uint32_t events, void* userData = nullptr) = 0;
    
    // 修改文件描述符关注的事件
    virtual bool modifyFd(int fd, uint32_t events, void* userData = nullptr) = 0;
    
    // 删除文件描述符
    virtual bool removeFd(int fd) = 0;
    
    // 等待事件发生
    // timeout: 超时时间（毫秒），-1表示永久等待，0表示非阻塞
    virtual std::vector<IOEvent> wait(int timeout = -1) = 0;
    
    // 设置事件回调函数
    virtual void setEventCallback(EventCallback callback) { eventCallback_ = callback; }
    
    // 运行事件循环
    virtual void run() = 0;
    
    // 停止事件循环
    virtual void stop() = 0;
    
    // 获取复用器类型名称
    virtual std::string getTypeName() const = 0;
    
    // 获取当前监听的文件描述符数量
    virtual size_t getFdCount() const = 0;
    
    // 获取最大支持的文件描述符数量
    virtual size_t getMaxFdCount() const = 0;
    
    // 是否正在运行
    virtual bool isRunning() const { return running_; }

protected:
    EventCallback eventCallback_;  // 事件回调函数
    bool running_ = false;         // 运行状态
    
    // 辅助函数：处理事件
    void handleEvents(const std::vector<IOEvent>& events) {
        if (eventCallback_) {
            for (const auto& event : events) {
                eventCallback_(event);
            }
        }
    }
};

// IO复用器类型枚举
enum class MultiplexerType {
    Select,    // select模型
    Poll,      // poll模型
    Epoll,     // epoll模型（Linux）
    Kqueue     // kqueue模型（BSD/macOS）
};

} // namespace IOMultiplexing 