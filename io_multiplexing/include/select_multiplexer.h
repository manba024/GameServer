#pragma once

#include "io_multiplexer.h"
#include <sys/select.h>
#include <map>

namespace IOMultiplexing {

// Select IO复用器实现
class SelectMultiplexer : public IOMultiplexer {
public:
    SelectMultiplexer();
    virtual ~SelectMultiplexer() = default;

    // 实现IOMultiplexer接口
    bool addFd(int fd, uint32_t events, void* userData = nullptr) override;
    bool modifyFd(int fd, uint32_t events, void* userData = nullptr) override;
    bool removeFd(int fd) override;
    std::vector<IOEvent> wait(int timeout = -1) override;
    void run() override;
    void stop() override;
    std::string getTypeName() const override { return "Select"; }
    size_t getFdCount() const override { return fdMap_.size(); }
    size_t getMaxFdCount() const override { return FD_SETSIZE; }

private:
    // 文件描述符信息
    struct FdInfo {
        uint32_t events;    // 关注的事件
        void* userData;     // 用户数据
        
        FdInfo(uint32_t e = 0, void* data = nullptr) 
            : events(e), userData(data) {}
    };
    
    std::map<int, FdInfo> fdMap_;  // 文件描述符映射表
    int maxFd_;                    // 最大文件描述符
    
    // 辅助函数
    void updateMaxFd();
    timeval* createTimeout(int timeoutMs);
    
    // fd_set集合
    fd_set readSet_;
    fd_set writeSet_;
    fd_set errorSet_;
};

} // namespace IOMultiplexing 