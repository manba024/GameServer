/*
 * @Author       : GameServer
 * @Date         : 2024-01-01
 * @Description  : IO复用封装 (跨平台: Linux epoll / macOS kqueue)
 */ 

#ifndef EPOLLER_H
#define EPOLLER_H

#ifdef __linux__
#include <sys/epoll.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/event.h>
#include <sys/time.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <errno.h>

class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();

    bool AddFd(int fd, uint32_t events);
    bool ModFd(int fd, uint32_t events);
    bool DelFd(int fd);
    int Wait(int timeoutMs = -1);
    int GetEventFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;
        
private:
#ifdef __linux__
    int epollFd_;
    std::vector<struct epoll_event> events_;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    int kqueueFd_;
    std::vector<struct kevent> events_;
#endif
};

#endif //EPOLLER_H 