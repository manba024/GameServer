#pragma once

#include "io_multiplexer.h"
#include <poll.h>
#include <unordered_map>

namespace IOMultiplexing {

// Poll IO复用器实现
class PollMultiplexer : public IOMultiplexer {
public:
    PollMultiplexer();
    virtual ~PollMultiplexer() = default;

    // 实现IOMultiplexer接口
    bool addFd(int fd, uint32_t events, void* userData = nullptr) override;
    bool modifyFd(int fd, uint32_t events, void* userData = nullptr) override;
    bool removeFd(int fd) override;
    std::vector<IOEvent> wait(int timeout = -1) override;
    void run() override;
    void stop() override;
    std::string getTypeName() const override { return "Poll"; }
    size_t getFdCount() const override { return fdInfoMap_.size(); }
    size_t getMaxFdCount() const override { return 65536; } // 理论上无限制，返回一个合理值

private:
    // 文件描述符信息
    struct FdInfo {
        size_t pollIndex;   // 在pollFds_数组中的索引
        void* userData;     // 用户数据
        
        FdInfo(size_t index = 0, void* data = nullptr) 
            : pollIndex(index), userData(data) {}
    };
    
    std::vector<pollfd> pollFds_;                    // poll文件描述符数组
    std::unordered_map<int, FdInfo> fdInfoMap_;      // fd到信息的映射
    
    // 辅助函数
    uint32_t convertFromPollEvents(short pollEvents);
    short convertToPollEvents(uint32_t events);
    void compactPollFds(); // 压缩pollFds_数组，移除无效项
};

} // namespace IOMultiplexing 