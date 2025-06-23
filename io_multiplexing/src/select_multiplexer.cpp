#include "../include/select_multiplexer.h"
#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <cerrno>
#include <cstring>

namespace IOMultiplexing {

SelectMultiplexer::SelectMultiplexer() : maxFd_(-1) {
    FD_ZERO(&readSet_);
    FD_ZERO(&writeSet_);
    FD_ZERO(&errorSet_);
}

bool SelectMultiplexer::addFd(int fd, uint32_t events, void* userData) {
    if (fd < 0 || fd >= FD_SETSIZE) {
        std::cerr << "Select: 无效的文件描述符 " << fd << std::endl;
        return false;
    }
    
    if (fdMap_.find(fd) != fdMap_.end()) {
        std::cerr << "Select: 文件描述符 " << fd << " 已存在" << std::endl;
        return false;
    }
    
    fdMap_[fd] = FdInfo(events, userData);
    updateMaxFd();
    
    return true;
}

bool SelectMultiplexer::modifyFd(int fd, uint32_t events, void* userData) {
    auto it = fdMap_.find(fd);
    if (it == fdMap_.end()) {
        std::cerr << "Select: 文件描述符 " << fd << " 不存在" << std::endl;
        return false;
    }
    
    it->second.events = events;
    it->second.userData = userData;
    return true;
}

bool SelectMultiplexer::removeFd(int fd) {
    auto it = fdMap_.find(fd);
    if (it == fdMap_.end()) {
        return false;
    }
    
    fdMap_.erase(it);
    updateMaxFd();
    return true;
}

std::vector<IOEvent> SelectMultiplexer::wait(int timeout) {
    std::vector<IOEvent> result;
    
    if (fdMap_.empty()) {
        return result;
    }
    
    // 清空fd_set并设置要监听的文件描述符
    FD_ZERO(&readSet_);
    FD_ZERO(&writeSet_);
    FD_ZERO(&errorSet_);
    
    for (const auto& pair : fdMap_) {
        int fd = pair.first;
        uint32_t events = pair.second.events;
        
        if (events & static_cast<uint32_t>(IOEventType::Read)) {
            FD_SET(fd, &readSet_);
        }
        if (events & static_cast<uint32_t>(IOEventType::Write)) {
            FD_SET(fd, &writeSet_);
        }
        // 错误事件总是监听
        FD_SET(fd, &errorSet_);
    }
    
    // 创建超时结构
    timeval* timeoutPtr = createTimeout(timeout);
    
    // 调用select
    int ready = select(maxFd_ + 1, &readSet_, &writeSet_, &errorSet_, timeoutPtr);
    
    if (ready < 0) {
        if (errno != EINTR) {
            std::cerr << "Select错误: " << strerror(errno) << std::endl;
        }
        return result;
    }
    
    if (ready == 0) {
        // 超时
        return result;
    }
    
    // 检查哪些文件描述符就绪
    for (const auto& pair : fdMap_) {
        int fd = pair.first;
        void* userData = pair.second.userData;
        uint32_t readyEvents = 0;
        
        if (FD_ISSET(fd, &readSet_)) {
            readyEvents |= static_cast<uint32_t>(IOEventType::Read);
        }
        if (FD_ISSET(fd, &writeSet_)) {
            readyEvents |= static_cast<uint32_t>(IOEventType::Write);
        }
        if (FD_ISSET(fd, &errorSet_)) {
            readyEvents |= static_cast<uint32_t>(IOEventType::Error);
        }
        
        if (readyEvents != 0) {
            result.emplace_back(fd, readyEvents, userData);
        }
    }
    
    return result;
}

void SelectMultiplexer::run() {
    running_ = true;
    std::cout << "Select IO复用器开始运行..." << std::endl;
    
    while (running_) {
        auto events = wait(1000); // 1秒超时
        handleEvents(events);
    }
    
    std::cout << "Select IO复用器停止运行" << std::endl;
}

void SelectMultiplexer::stop() {
    running_ = false;
}

void SelectMultiplexer::updateMaxFd() {
    maxFd_ = -1;
    for (const auto& pair : fdMap_) {
        if (pair.first > maxFd_) {
            maxFd_ = pair.first;
        }
    }
}

timeval* SelectMultiplexer::createTimeout(int timeoutMs) {
    static timeval tv;
    
    if (timeoutMs < 0) {
        return nullptr; // 永久等待
    }
    
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    return &tv;
}

} // namespace IOMultiplexing 