#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

#include "../include/kqueue_multiplexer.h"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <unistd.h>

namespace IOMultiplexing {

KqueueMultiplexer::KqueueMultiplexer(int maxEvents) : maxEvents_(maxEvents) {
    kqueueFd_ = kqueue();
    if (kqueueFd_ < 0) {
        std::cerr << "Kqueue: 创建kqueue失败: " << strerror(errno) << std::endl;
        throw std::runtime_error("Failed to create kqueue");
    }
    
    events_.resize(maxEvents_);
    std::cout << "Kqueue IO复用器创建成功，最大事件数: " << maxEvents_ << std::endl;
}

KqueueMultiplexer::~KqueueMultiplexer() {
    if (kqueueFd_ >= 0) {
        close(kqueueFd_);
    }
}

bool KqueueMultiplexer::addFd(int fd, uint32_t events, void* userData) {
    if (fd < 0) {
        std::cerr << "Kqueue: 无效的文件描述符 " << fd << std::endl;
        return false;
    }
    
    if (fdInfoMap_.find(fd) != fdInfoMap_.end()) {
        std::cerr << "Kqueue: 文件描述符 " << fd << " 已存在" << std::endl;
        return false;
    }
    
    bool success = true;
    
    // 添加读事件
    if (events & static_cast<uint32_t>(IOEventType::Read)) {
        if (!addKqueueEvent(fd, EVFILT_READ, EV_ADD | EV_ENABLE, userData)) {
            success = false;
        }
    }
    
    // 添加写事件
    if (events & static_cast<uint32_t>(IOEventType::Write)) {
        if (!addKqueueEvent(fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, userData)) {
            success = false;
        }
    }
    
    if (success) {
        fdInfoMap_[fd] = FdInfo(events, userData);
    }
    
    return success;
}

bool KqueueMultiplexer::modifyFd(int fd, uint32_t events, void* userData) {
    auto it = fdInfoMap_.find(fd);
    if (it == fdInfoMap_.end()) {
        std::cerr << "Kqueue: 文件描述符 " << fd << " 不存在" << std::endl;
        return false;
    }
    
    uint32_t oldEvents = it->second.events;
    bool success = true;
    
    // 处理读事件的变化
    bool oldRead = oldEvents & static_cast<uint32_t>(IOEventType::Read);
    bool newRead = events & static_cast<uint32_t>(IOEventType::Read);
    
    if (oldRead && !newRead) {
        // 删除读事件
        if (!removeKqueueEvent(fd, EVFILT_READ)) {
            success = false;
        }
    } else if (!oldRead && newRead) {
        // 添加读事件
        if (!addKqueueEvent(fd, EVFILT_READ, EV_ADD | EV_ENABLE, userData)) {
            success = false;
        }
    }
    
    // 处理写事件的变化
    bool oldWrite = oldEvents & static_cast<uint32_t>(IOEventType::Write);
    bool newWrite = events & static_cast<uint32_t>(IOEventType::Write);
    
    if (oldWrite && !newWrite) {
        // 删除写事件
        if (!removeKqueueEvent(fd, EVFILT_WRITE)) {
            success = false;
        }
    } else if (!oldWrite && newWrite) {
        // 添加写事件
        if (!addKqueueEvent(fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, userData)) {
            success = false;
        }
    }
    
    if (success) {
        it->second.events = events;
        it->second.userData = userData;
    }
    
    return success;
}

bool KqueueMultiplexer::removeFd(int fd) {
    auto it = fdInfoMap_.find(fd);
    if (it == fdInfoMap_.end()) {
        return false;
    }
    
    uint32_t events = it->second.events;
    bool success = true;
    
    // 删除读事件
    if (events & static_cast<uint32_t>(IOEventType::Read)) {
        if (!removeKqueueEvent(fd, EVFILT_READ)) {
            success = false;
        }
    }
    
    // 删除写事件
    if (events & static_cast<uint32_t>(IOEventType::Write)) {
        if (!removeKqueueEvent(fd, EVFILT_WRITE)) {
            success = false;
        }
    }
    
    fdInfoMap_.erase(it);
    return success;
}

std::vector<IOEvent> KqueueMultiplexer::wait(int timeout) {
    std::vector<IOEvent> result;
    
    if (fdInfoMap_.empty()) {
        return result;
    }
    
    // 创建超时结构
    timespec* timeoutPtr = createTimeout(timeout);
    
    // 调用kevent
    int ready = kevent(kqueueFd_, nullptr, 0, events_.data(), maxEvents_, timeoutPtr);
    
    if (ready < 0) {
        if (errno != EINTR) {
            std::cerr << "Kqueue错误: " << strerror(errno) << std::endl;
        }
        return result;
    }
    
    if (ready == 0) {
        // 超时
        return result;
    }
    
    // 处理就绪的事件
    for (int i = 0; i < ready; i++) {
        const struct kevent& ev = events_[i];
        int fd = static_cast<int>(ev.ident);
        
        auto it = fdInfoMap_.find(fd);
        if (it != fdInfoMap_.end()) {
            uint32_t readyEvents = convertFromKqueueEvents(ev);
            void* userData = it->second.userData;
            result.emplace_back(fd, readyEvents, userData);
        }
    }
    
    return result;
}

void KqueueMultiplexer::run() {
    running_ = true;
    std::cout << "Kqueue IO复用器开始运行..." << std::endl;
    
    while (running_) {
        auto events = wait(1000); // 1秒超时
        handleEvents(events);
    }
    
    std::cout << "Kqueue IO复用器停止运行" << std::endl;
}

void KqueueMultiplexer::stop() {
    running_ = false;
}

uint32_t KqueueMultiplexer::convertFromKqueueEvents(const struct kevent& event) {
    uint32_t events = 0;
    
    if (event.filter == EVFILT_READ) {
        events |= static_cast<uint32_t>(IOEventType::Read);
    }
    if (event.filter == EVFILT_WRITE) {
        events |= static_cast<uint32_t>(IOEventType::Write);
    }
    if (event.flags & EV_EOF) {
        events |= static_cast<uint32_t>(IOEventType::HangUp);
    }
    if (event.flags & EV_ERROR) {
        events |= static_cast<uint32_t>(IOEventType::Error);
    }
    
    return events;
}

bool KqueueMultiplexer::addKqueueEvent(int fd, int16_t filter, uint16_t flags, void* userData) {
    struct kevent ev;
    EV_SET(&ev, fd, filter, flags, 0, 0, userData);
    
    if (kevent(kqueueFd_, &ev, 1, nullptr, 0, nullptr) < 0) {
        std::cerr << "Kqueue: 添加事件失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    return true;
}

bool KqueueMultiplexer::removeKqueueEvent(int fd, int16_t filter) {
    struct kevent ev;
    EV_SET(&ev, fd, filter, EV_DELETE, 0, 0, nullptr);
    
    if (kevent(kqueueFd_, &ev, 1, nullptr, 0, nullptr) < 0) {
        // 删除不存在的事件可能会失败，这通常不是严重错误
        if (errno != ENOENT) {
            std::cerr << "Kqueue: 删除事件失败: " << strerror(errno) << std::endl;
            return false;
        }
    }
    
    return true;
}

timespec* KqueueMultiplexer::createTimeout(int timeoutMs) {
    static timespec ts;
    
    if (timeoutMs < 0) {
        return nullptr; // 永久等待
    }
    
    ts.tv_sec = timeoutMs / 1000;
    ts.tv_nsec = (timeoutMs % 1000) * 1000000;
    return &ts;
}

} // namespace IOMultiplexing

#endif // BSD/macOS platforms 