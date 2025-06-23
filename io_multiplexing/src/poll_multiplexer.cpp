#include "../include/poll_multiplexer.h"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <algorithm>

namespace IOMultiplexing {

PollMultiplexer::PollMultiplexer() {
    // poll不需要特殊初始化
}

bool PollMultiplexer::addFd(int fd, uint32_t events, void* userData) {
    if (fd < 0) {
        std::cerr << "Poll: 无效的文件描述符 " << fd << std::endl;
        return false;
    }
    
    if (fdInfoMap_.find(fd) != fdInfoMap_.end()) {
        std::cerr << "Poll: 文件描述符 " << fd << " 已存在" << std::endl;
        return false;
    }
    
    // 添加到pollfd数组
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = convertToPollEvents(events);
    pfd.revents = 0;
    
    size_t index = pollFds_.size();
    pollFds_.push_back(pfd);
    fdInfoMap_[fd] = FdInfo(index, userData);
    
    return true;
}

bool PollMultiplexer::modifyFd(int fd, uint32_t events, void* userData) {
    auto it = fdInfoMap_.find(fd);
    if (it == fdInfoMap_.end()) {
        std::cerr << "Poll: 文件描述符 " << fd << " 不存在" << std::endl;
        return false;
    }
    
    size_t index = it->second.pollIndex;
    if (index < pollFds_.size()) {
        pollFds_[index].events = convertToPollEvents(events);
        it->second.userData = userData;
    }
    
    return true;
}

bool PollMultiplexer::removeFd(int fd) {
    auto it = fdInfoMap_.find(fd);
    if (it == fdInfoMap_.end()) {
        return false;
    }
    
    size_t index = it->second.pollIndex;
    if (index < pollFds_.size()) {
        // 标记为无效，稍后压缩
        pollFds_[index].fd = -1;
    }
    
    fdInfoMap_.erase(it);
    
    // 如果删除的项目过多，压缩数组
    if (fdInfoMap_.size() < pollFds_.size() / 2) {
        compactPollFds();
    }
    
    return true;
}

std::vector<IOEvent> PollMultiplexer::wait(int timeout) {
    std::vector<IOEvent> result;
    
    if (pollFds_.empty()) {
        return result;
    }
    
    // 调用poll
    int ready = poll(pollFds_.data(), pollFds_.size(), timeout);
    
    if (ready < 0) {
        if (errno != EINTR) {
            std::cerr << "Poll错误: " << strerror(errno) << std::endl;
        }
        return result;
    }
    
    if (ready == 0) {
        // 超时
        return result;
    }
    
    // 检查就绪的文件描述符
    for (size_t i = 0; i < pollFds_.size() && ready > 0; i++) {
        if (pollFds_[i].revents != 0) {
            int fd = pollFds_[i].fd;
            if (fd >= 0) { // 有效的fd
                auto it = fdInfoMap_.find(fd);
                if (it != fdInfoMap_.end()) {
                    uint32_t events = convertFromPollEvents(pollFds_[i].revents);
                    result.emplace_back(fd, events, it->second.userData);
                    ready--;
                }
            }
            pollFds_[i].revents = 0; // 清除事件
        }
    }
    
    return result;
}

void PollMultiplexer::run() {
    running_ = true;
    std::cout << "Poll IO复用器开始运行..." << std::endl;
    
    while (running_) {
        auto events = wait(1000); // 1秒超时
        handleEvents(events);
    }
    
    std::cout << "Poll IO复用器停止运行" << std::endl;
}

void PollMultiplexer::stop() {
    running_ = false;
}

uint32_t PollMultiplexer::convertFromPollEvents(short pollEvents) {
    uint32_t events = 0;
    
    if (pollEvents & POLLIN) {
        events |= static_cast<uint32_t>(IOEventType::Read);
    }
    if (pollEvents & POLLOUT) {
        events |= static_cast<uint32_t>(IOEventType::Write);
    }
    if (pollEvents & (POLLERR | POLLNVAL)) {
        events |= static_cast<uint32_t>(IOEventType::Error);
    }
    if (pollEvents & POLLHUP) {
        events |= static_cast<uint32_t>(IOEventType::HangUp);
    }
    
    return events;
}

short PollMultiplexer::convertToPollEvents(uint32_t events) {
    short pollEvents = 0;
    
    if (events & static_cast<uint32_t>(IOEventType::Read)) {
        pollEvents |= POLLIN;
    }
    if (events & static_cast<uint32_t>(IOEventType::Write)) {
        pollEvents |= POLLOUT;
    }
    // POLLERR总是隐式监听
    
    return pollEvents;
}

void PollMultiplexer::compactPollFds() {
    std::vector<pollfd> newPollFds;
    
    for (size_t i = 0; i < pollFds_.size(); i++) {
        if (pollFds_[i].fd >= 0) {
            // 更新索引映射
            auto it = fdInfoMap_.find(pollFds_[i].fd);
            if (it != fdInfoMap_.end()) {
                it->second.pollIndex = newPollFds.size();
            }
            newPollFds.push_back(pollFds_[i]);
        }
    }
    
    pollFds_ = std::move(newPollFds);
}

} // namespace IOMultiplexing 