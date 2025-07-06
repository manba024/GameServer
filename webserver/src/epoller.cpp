/*
 * @Author       : GameServer
 * @Date         : 2024-01-01
 * @Description  : IO复用封装实现 (跨平台)
 */ 

#include "../include/epoller.h"

#ifdef __APPLE__
// 在macOS上定义epoll兼容的宏
#define EPOLLIN 0x001
#define EPOLLOUT 0x004
#define EPOLLERR 0x008
#define EPOLLHUP 0x010
#define EPOLLRDHUP 0x2000
#define EPOLLET 0x80000000
#endif

Epoller::Epoller(int maxEvent):events_(maxEvent){
#ifdef __linux__
    epollFd_ = epoll_create(512);
    assert(epollFd_ >= 0 && events_.size() > 0);
#elif defined(__APPLE__) || defined(__FreeBSD__)
    kqueueFd_ = kqueue();
    assert(kqueueFd_ >= 0 && events_.size() > 0);
#endif
}

Epoller::~Epoller() {
#ifdef __linux__
    close(epollFd_);
#elif defined(__APPLE__) || defined(__FreeBSD__)
    close(kqueueFd_);
#endif
}

bool Epoller::AddFd(int fd, uint32_t events) {
#ifdef __linux__
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
#elif defined(__APPLE__) || defined(__FreeBSD__)
    if(fd < 0) return false;
    struct kevent ev;
    if(events & EPOLLIN) {
        EV_SET(&ev, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if(kevent(kqueueFd_, &ev, 1, NULL, 0, NULL) == -1) return false;
    }
    if(events & EPOLLOUT) {
        EV_SET(&ev, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        if(kevent(kqueueFd_, &ev, 1, NULL, 0, NULL) == -1) return false;
    }
    return true;
#endif
}

bool Epoller::ModFd(int fd, uint32_t events) {
#ifdef __linux__
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);
#elif defined(__APPLE__) || defined(__FreeBSD__)
    // kqueue doesn't have modify, we delete and add
    DelFd(fd);
    return AddFd(fd, events);
#endif
}

bool Epoller::DelFd(int fd) {
#ifdef __linux__
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);
#elif defined(__APPLE__) || defined(__FreeBSD__)
    if(fd < 0) return false;
    struct kevent ev;
    EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    kevent(kqueueFd_, &ev, 1, NULL, 0, NULL);
    EV_SET(&ev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    kevent(kqueueFd_, &ev, 1, NULL, 0, NULL);
    return true;
#endif
}

int Epoller::Wait(int timeoutMs) {
#ifdef __linux__
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
#elif defined(__APPLE__) || defined(__FreeBSD__)
    struct timespec timeout;
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_nsec = (timeoutMs % 1000) * 1000000;
    return kevent(kqueueFd_, NULL, 0, &events_[0], static_cast<int>(events_.size()), 
                  timeoutMs < 0 ? NULL : &timeout);
#endif
}

int Epoller::GetEventFd(size_t i) const {
    assert(i < events_.size() && i >= 0);
#ifdef __linux__
    return events_[i].data.fd;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    return static_cast<int>(events_[i].ident);
#endif
}

uint32_t Epoller::GetEvents(size_t i) const {
    assert(i < events_.size() && i >= 0);
#ifdef __linux__
    return events_[i].events;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    uint32_t events = 0;
    if(events_[i].filter == EVFILT_READ) events |= EPOLLIN;
    if(events_[i].filter == EVFILT_WRITE) events |= EPOLLOUT;
    if(events_[i].flags & EV_EOF) events |= EPOLLHUP;
    if(events_[i].flags & EV_ERROR) events |= EPOLLERR;
    return events;
#endif
} 