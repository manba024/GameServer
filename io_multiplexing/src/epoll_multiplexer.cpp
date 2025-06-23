#ifdef __linux__

#include "../include/epoll_multiplexer.h"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <unistd.h>

namespace IOMultiplexing {

EpollMultiplexer::EpollMultiplexer(int maxEvents, EpollTriggerMode defaultMode) 
    : maxEvents_(maxEvents), defaultTriggerMode_(defaultMode) {
    epollFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd_ < 0) {
        std::cerr << "Epoll: 创建epoll失败: " << strerror(errno) << std::endl;
        throw std::runtime_error("Failed to create epoll");
    }
    
    events_.resize(maxEvents_);
    std::cout << "Epoll IO复用器创建成功，最大事件数: " << maxEvents_ 
              << ", 默认触发模式: " << (defaultMode == EpollTriggerMode::LevelTriggered ? "LT" : "ET") 
              << std::endl;
}

EpollMultiplexer::~EpollMultiplexer() {
    if (epollFd_ >= 0) {
        close(epollFd_);
    }
}

bool EpollMultiplexer::addFd(int fd, uint32_t events, void* userData) {
    return addFd(fd, events, defaultTriggerMode_, userData);
}

bool EpollMultiplexer::addFd(int fd, uint32_t events, EpollTriggerMode mode, void* userData) {
    if (fd < 0) {
        std::cerr << "Epoll: 无效的文件描述符 " << fd << std::endl;
        return false;
    }
    
    if (fdInfoMap_.find(fd) != fdInfoMap_.end()) {
        std::cerr << "Epoll: 文件描述符 " << fd << " 已存在" << std::endl;
        return false;
    }
    
    epoll_event ev;
    ev.events = convertToEpollEvents(events, mode);
    ev.data.fd = fd;
    ev.data.ptr = userData;
    
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        std::cerr << "Epoll: 添加文件描述符失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    fdInfoMap_[fd] = FdInfo(userData, mode);
    std::cout << "Epoll: 添加fd=" << fd << ", 触发模式=" << (mode == EpollTriggerMode::LevelTriggered ? "LT" : "ET") << std::endl;
    return true;
}

bool EpollMultiplexer::modifyFd(int fd, uint32_t events, void* userData) {
    auto it = fdInfoMap_.find(fd);
    if (it == fdInfoMap_.end()) {
        std::cerr << "Epoll: 文件描述符 " << fd << " 不存在" << std::endl;
        return false;
    }
    
    // 保持原有的触发模式
    return modifyFd(fd, events, it->second.triggerMode, userData);
}

bool EpollMultiplexer::modifyFd(int fd, uint32_t events, EpollTriggerMode mode, void* userData) {
    auto it = fdInfoMap_.find(fd);
    if (it == fdInfoMap_.end()) {
        std::cerr << "Epoll: 文件描述符 " << fd << " 不存在" << std::endl;
        return false;
    }
    
    epoll_event ev;
    ev.events = convertToEpollEvents(events, mode);
    ev.data.fd = fd;
    ev.data.ptr = userData;
    
    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        std::cerr << "Epoll: 修改文件描述符失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    it->second.userData = userData;
    it->second.triggerMode = mode;
    return true;
}

bool EpollMultiplexer::removeFd(int fd) {
    auto it = fdInfoMap_.find(fd);
    if (it == fdInfoMap_.end()) {
        return false;
    }
    
    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        std::cerr << "Epoll: 删除文件描述符失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    fdInfoMap_.erase(it);
    return true;
}

EpollTriggerMode EpollMultiplexer::getFdTriggerMode(int fd) const {
    auto it = fdInfoMap_.find(fd);
    if (it != fdInfoMap_.end()) {
        return it->second.triggerMode;
    }
    return defaultTriggerMode_;
}

std::vector<IOEvent> EpollMultiplexer::wait(int timeout) {
    std::vector<IOEvent> result;
    
    if (fdInfoMap_.empty()) {
        return result;
    }
    
    // 调用epoll_wait
    int ready = epoll_wait(epollFd_, events_.data(), maxEvents_, timeout);
    
    if (ready < 0) {
        if (errno != EINTR) {
            std::cerr << "Epoll错误: " << strerror(errno) << std::endl;
        }
        return result;
    }
    
    if (ready == 0) {
        // 超时
        return result;
    }
    
    // 处理就绪的事件
    for (int i = 0; i < ready; i++) {
        const epoll_event& ev = events_[i];
        int fd = ev.data.fd;
        
        auto it = fdInfoMap_.find(fd);
        if (it != fdInfoMap_.end()) {
            uint32_t readyEvents = convertFromEpollEvents(ev.events);
            void* userData = it->second.userData;
            result.emplace_back(fd, readyEvents, userData);
        }
    }
    
    return result;
}

void EpollMultiplexer::run() {
    running_ = true;
    std::cout << "Epoll IO复用器开始运行..." << std::endl;
    
    while (running_) {
        auto events = wait(1000); // 1秒超时
        handleEvents(events);
    }
    
    std::cout << "Epoll IO复用器停止运行" << std::endl;
}

void EpollMultiplexer::stop() {
    running_ = false;
}

uint32_t EpollMultiplexer::convertFromEpollEvents(uint32_t epollEvents) {
    uint32_t events = 0;
    
    if (epollEvents & EPOLLIN) {
        events |= static_cast<uint32_t>(IOEventType::Read);
    }
    if (epollEvents & EPOLLOUT) {
        events |= static_cast<uint32_t>(IOEventType::Write);
    }
    if (epollEvents & EPOLLERR) {
        events |= static_cast<uint32_t>(IOEventType::Error);
    }
    if (epollEvents & EPOLLHUP) {
        events |= static_cast<uint32_t>(IOEventType::HangUp);
    }
    
    return events;
}

uint32_t EpollMultiplexer::convertToEpollEvents(uint32_t events, EpollTriggerMode mode) {
    uint32_t epollEvents = 0;
    
    if (events & static_cast<uint32_t>(IOEventType::Read)) {
        epollEvents |= EPOLLIN;
    }
    if (events & static_cast<uint32_t>(IOEventType::Write)) {
        epollEvents |= EPOLLOUT;
    }
    
    // 根据触发模式设置标志
    if (mode == EpollTriggerMode::EdgeTriggered) {
        epollEvents |= EPOLLET;  // 边缘触发
    }
    // LT模式不需要额外标志，这是epoll的默认行为
    
    return epollEvents;
}

} // namespace IOMultiplexing

#endif // __linux__ 