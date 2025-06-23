#include "../include/multiplexer_factory.h"
#include "../include/select_multiplexer.h"
#include "../include/poll_multiplexer.h"

#ifdef __linux__
#include "../include/epoll_multiplexer.h"
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include "../include/kqueue_multiplexer.h"
#endif

#include <iostream>
#include <algorithm>

namespace IOMultiplexing {

std::unique_ptr<IOMultiplexer> MultiplexerFactory::create(MultiplexerType type, int maxEvents) {
    switch (type) {
        case MultiplexerType::Select:
            return std::unique_ptr<IOMultiplexer>(new SelectMultiplexer());
            
        case MultiplexerType::Poll:
            return std::unique_ptr<IOMultiplexer>(new PollMultiplexer());
            
        case MultiplexerType::Epoll:
#ifdef __linux__
            return std::unique_ptr<IOMultiplexer>(new EpollMultiplexer(maxEvents));
#else
            std::cerr << "Epoll只在Linux系统上支持" << std::endl;
            return nullptr;
#endif
            
        case MultiplexerType::Kqueue:
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
            return std::unique_ptr<IOMultiplexer>(new KqueueMultiplexer(maxEvents));
#else
            std::cerr << "Kqueue只在BSD/macOS系统上支持" << std::endl;
            return nullptr;
#endif
            
        default:
            std::cerr << "不支持的IO复用器类型" << std::endl;
            return nullptr;
    }
}

std::unique_ptr<IOMultiplexer> MultiplexerFactory::createBest(int maxEvents) {
    // 按性能优先级选择最佳的IO复用器
    auto recommendedTypes = getRecommendedTypes();
    
    for (auto type : recommendedTypes) {
        if (isSupported(type)) {
            auto multiplexer = create(type, maxEvents);
            if (multiplexer) {
                std::cout << "选择了 " << multiplexer->getTypeName() << " IO复用器" << std::endl;
                return multiplexer;
            }
        }
    }
    
    // 如果所有推荐的都不支持，回退到Select
    std::cout << "回退到 Select IO复用器" << std::endl;
    return create(MultiplexerType::Select, maxEvents);
}

std::vector<MultiplexerType> MultiplexerFactory::getSupportedTypes() {
    std::vector<MultiplexerType> supported;
    
    // Select和Poll在所有POSIX系统上都支持
    supported.push_back(MultiplexerType::Select);
    supported.push_back(MultiplexerType::Poll);
    
#ifdef __linux__
    supported.push_back(MultiplexerType::Epoll);
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    supported.push_back(MultiplexerType::Kqueue);
#endif
    
    return supported;
}

bool MultiplexerFactory::isSupported(MultiplexerType type) {
    auto supported = getSupportedTypes();
    return std::find(supported.begin(), supported.end(), type) != supported.end();
}

std::string MultiplexerFactory::getTypeName(MultiplexerType type) {
    switch (type) {
        case MultiplexerType::Select: return "Select";
        case MultiplexerType::Poll: return "Poll";
        case MultiplexerType::Epoll: return "Epoll";
        case MultiplexerType::Kqueue: return "Kqueue";
        default: return "Unknown";
    }
}

std::vector<MultiplexerType> MultiplexerFactory::getRecommendedTypes() {
    std::vector<MultiplexerType> recommended;
    
#ifdef __linux__
    // Linux上优先使用Epoll
    recommended.push_back(MultiplexerType::Epoll);
    recommended.push_back(MultiplexerType::Poll);
    recommended.push_back(MultiplexerType::Select);
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    // BSD/macOS上优先使用Kqueue
    recommended.push_back(MultiplexerType::Kqueue);
    recommended.push_back(MultiplexerType::Poll);
    recommended.push_back(MultiplexerType::Select);
#else
    // 其他系统
    recommended.push_back(MultiplexerType::Poll);
    recommended.push_back(MultiplexerType::Select);
#endif
    
    return recommended;
}

// 便捷函数实现
std::unique_ptr<IOMultiplexer> createMultiplexer(MultiplexerType type, int maxEvents) {
    return MultiplexerFactory::create(type, maxEvents);
}

std::unique_ptr<IOMultiplexer> createBestMultiplexer(int maxEvents) {
    return MultiplexerFactory::createBest(maxEvents);
}

} // namespace IOMultiplexing 