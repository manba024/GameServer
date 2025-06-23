#pragma once

#include "io_multiplexer.h"

#ifdef __linux__
#include <sys/epoll.h>
#include <unordered_map>

namespace IOMultiplexing {

// Epoll触发模式
enum class EpollTriggerMode {
    LevelTriggered,  // LT 水平触发（默认）
    EdgeTriggered    // ET 边缘触发
};

// Epoll IO复用器实现（Linux特有）
class EpollMultiplexer : public IOMultiplexer {
public:
    EpollMultiplexer(int maxEvents = 1024, EpollTriggerMode defaultMode = EpollTriggerMode::LevelTriggered);
    virtual ~EpollMultiplexer();

    // 实现IOMultiplexer接口
    bool addFd(int fd, uint32_t events, void* userData = nullptr) override;
    bool modifyFd(int fd, uint32_t events, void* userData = nullptr) override;
    bool removeFd(int fd) override;
    std::vector<IOEvent> wait(int timeout = -1) override;
    void run() override;
    void stop() override;
    std::string getTypeName() const override { return "Epoll"; }
    size_t getFdCount() const override { return fdInfoMap_.size(); }
    size_t getMaxFdCount() const override { return 1000000; } // epoll理论上支持很多fd

    // 扩展接口：支持指定触发模式
    bool addFd(int fd, uint32_t events, EpollTriggerMode mode, void* userData = nullptr);
    bool modifyFd(int fd, uint32_t events, EpollTriggerMode mode, void* userData = nullptr);
    
    // 设置默认触发模式
    void setDefaultTriggerMode(EpollTriggerMode mode) { defaultTriggerMode_ = mode; }
    EpollTriggerMode getDefaultTriggerMode() const { return defaultTriggerMode_; }
    
    // 获取fd的触发模式
    EpollTriggerMode getFdTriggerMode(int fd) const;

private:
    // 文件描述符信息
    struct FdInfo {
        void* userData;              // 用户数据
        EpollTriggerMode triggerMode; // 触发模式
        
        FdInfo(void* data = nullptr, EpollTriggerMode mode = EpollTriggerMode::LevelTriggered) 
            : userData(data), triggerMode(mode) {}
    };
    
    int epollFd_;                               // epoll文件描述符
    std::vector<epoll_event> events_;           // 事件数组
    std::unordered_map<int, FdInfo> fdInfoMap_; // fd信息映射
    int maxEvents_;                             // 最大事件数
    EpollTriggerMode defaultTriggerMode_;       // 默认触发模式
    
    // 辅助函数
    uint32_t convertFromEpollEvents(uint32_t epollEvents);
    uint32_t convertToEpollEvents(uint32_t events, EpollTriggerMode mode);
};

} // namespace IOMultiplexing

#endif // __linux__ 