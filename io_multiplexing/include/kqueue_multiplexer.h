#pragma once

#include "io_multiplexer.h"

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <sys/event.h>
#include <unordered_map>

namespace IOMultiplexing {

// Kqueue IO复用器实现（macOS/BSD特有）
class KqueueMultiplexer : public IOMultiplexer {
public:
    KqueueMultiplexer(int maxEvents = 1024);
    virtual ~KqueueMultiplexer();

    // 实现IOMultiplexer接口
    bool addFd(int fd, uint32_t events, void* userData = nullptr) override;
    bool modifyFd(int fd, uint32_t events, void* userData = nullptr) override;
    bool removeFd(int fd) override;
    std::vector<IOEvent> wait(int timeout = -1) override;
    void run() override;
    void stop() override;
    std::string getTypeName() const override { return "Kqueue"; }
    size_t getFdCount() const override { return fdInfoMap_.size(); }
    size_t getMaxFdCount() const override { return 1000000; } // kqueue理论上支持很多fd

private:
    // 文件描述符信息
    struct FdInfo {
        uint32_t events;    // 当前关注的事件
        void* userData;     // 用户数据
        
        FdInfo(uint32_t e = 0, void* data = nullptr) 
            : events(e), userData(data) {}
    };
    
    int kqueueFd_;                              // kqueue文件描述符
    std::vector<struct kevent> events_;         // 事件数组
    std::unordered_map<int, FdInfo> fdInfoMap_; // fd信息映射
    int maxEvents_;                             // 最大事件数
    
    // 辅助函数
    uint32_t convertFromKqueueEvents(const struct kevent& event);
    bool addKqueueEvent(int fd, int16_t filter, uint16_t flags, void* userData);
    bool removeKqueueEvent(int fd, int16_t filter);
    timespec* createTimeout(int timeoutMs);
};

} // namespace IOMultiplexing

#endif // BSD/macOS platforms 