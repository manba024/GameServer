#ifdef __linux__

#include "../include/multiplexer_factory.h"
#include "../include/epoll_multiplexer.h"
#include "../../threadpool/include/thread_pool_factory.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/resource.h>
#include <signal.h>

using namespace IOMultiplexing;
using namespace ThreadPool;

// 全局统计信息
struct ServerStats {
    std::atomic<uint64_t> totalConnections{0};
    std::atomic<uint64_t> activeConnections{0};
    std::atomic<uint64_t> totalMessages{0};
    std::atomic<uint64_t> totalBytes{0};
    std::atomic<uint64_t> errors{0};
    
    void printStats() const {
        std::cout << "=== 服务器统计 ===\n";
        std::cout << "总连接数: " << totalConnections.load() << "\n";
        std::cout << "活跃连接: " << activeConnections.load() << "\n";
        std::cout << "总消息数: " << totalMessages.load() << "\n";
        std::cout << "总字节数: " << totalBytes.load() << "\n";
        std::cout << "错误次数: " << errors.load() << "\n";
    }
};

// 连接管理
struct Connection {
    int fd;
    std::string buffer;
    std::chrono::steady_clock::time_point lastActivity;
    
    Connection(int f) : fd(f), lastActivity(std::chrono::steady_clock::now()) {}
};

// 设置系统限制
void setupSystemLimits() {
    std::cout << "=== 设置系统限制 ===\n";
    
    // 设置文件描述符限制
    struct rlimit rlim;
    rlim.rlim_cur = 1048576;  // 1M文件描述符
    rlim.rlim_max = 1048576;
    
    if (setrlimit(RLIMIT_NOFILE, &rlim) == 0) {
        std::cout << "✅ 文件描述符限制设置为: " << rlim.rlim_cur << "\n";
    } else {
        std::cerr << "❌ 设置文件描述符限制失败: " << strerror(errno) << "\n";
        std::cout << "💡 请使用 'ulimit -n 1048576' 或修改 /etc/security/limits.conf\n";
    }
    
    // 忽略SIGPIPE信号
    signal(SIGPIPE, SIG_IGN);
    
    // 显示当前限制
    getrlimit(RLIMIT_NOFILE, &rlim);
    std::cout << "当前文件描述符限制: " << rlim.rlim_cur << "/" << rlim.rlim_max << "\n";
}

// 工具函数
bool setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return flags != -1 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

bool setReuseAddr(int fd) {
    int opt = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
}

bool setReusePort(int fd) {
    int opt = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == 0;
}

int createServerSocket(int port) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) return -1;
    
    if (!setReuseAddr(serverFd) || !setReusePort(serverFd) || !setNonBlocking(serverFd)) {
        close(serverFd);
        return -1;
    }
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(serverFd, (sockaddr*)&addr, sizeof(addr)) < 0 || listen(serverFd, 65535) < 0) {
        close(serverFd);
        return -1;
    }
    
    return serverFd;
}

// 模式1: 单Epoll + 线程池模式 (Reactor模式)
class SingleEpollReactor {
private:
    std::unique_ptr<EpollMultiplexer> multiplexer_;
    std::unique_ptr<IThreadPool> threadPool_;
    std::unordered_map<int, std::unique_ptr<Connection>> connections_;
    std::mutex connectionsMutex_;
    ServerStats stats_;
    std::atomic<bool> running_{false};
    
public:
    SingleEpollReactor() {
        // 创建ET模式的epoll，提高性能
        multiplexer_ = std::make_unique<EpollMultiplexer>(65536, EpollTriggerMode::EdgeTriggered);
        
        // 创建固定大小线程池
        ThreadPoolConfig config;
        config.threadCount = std::thread::hardware_concurrency() * 2;
        config.maxQueueSize = 100000;
        config.rejectionPolicy = RejectionPolicy::Block;  // 阻塞等待而不是丢弃
        
        threadPool_ = ThreadPoolFactory::create(ThreadPoolType::Fixed, config);
        
        std::cout << "Reactor模式初始化: " << config.threadCount << "个工作线程\n";
    }
    
    void start(int port) {
        int serverFd = createServerSocket(port);
        if (serverFd < 0) {
            throw std::runtime_error("创建服务器socket失败");
        }
        
        std::cout << "Reactor服务器启动在端口 " << port << "\n";
        
        // 添加服务器socket
        multiplexer_->addFd(serverFd, static_cast<uint32_t>(IOEventType::Read));
        
        // 设置事件回调
        multiplexer_->setEventCallback([this, serverFd](const IOEvent& event) {
            if (event.fd == serverFd) {
                handleAccept(serverFd);
            } else {
                handleClientEvent(event);
            }
        });
        
        running_ = true;
        
        // 启动统计线程
        std::thread([this]() {
            while (running_) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                stats_.printStats();
                std::cout << "线程池状态: " << threadPool_->getStats().activeThreads 
                         << "/" << threadPool_->getStats().totalThreads << " 活跃\n\n";
            }
        }).detach();
        
        // 运行主循环
        multiplexer_->run();
        close(serverFd);
    }
    
    void stop() {
        running_ = false;
        multiplexer_->stop();
    }
    
private:
    void handleAccept(int serverFd) {
        // ET模式需要循环accept
        while (true) {
            sockaddr_in clientAddr{};
            socklen_t addrLen = sizeof(clientAddr);
            int clientFd = accept(serverFd, (sockaddr*)&clientAddr, &addrLen);
            
            if (clientFd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;  // 没有更多连接
                } else {
                    stats_.errors++;
                    break;
                }
            }
            
            if (!setNonBlocking(clientFd)) {
                close(clientFd);
                stats_.errors++;
                continue;
            }
            
            // 添加到epoll
            if (!multiplexer_->addFd(clientFd, static_cast<uint32_t>(IOEventType::Read))) {
                close(clientFd);
                stats_.errors++;
                continue;
            }
            
            // 创建连接对象
            {
                std::lock_guard<std::mutex> lock(connectionsMutex_);
                connections_[clientFd] = std::make_unique<Connection>(clientFd);
            }
            
            stats_.totalConnections++;
            stats_.activeConnections++;
        }
    }
    
    void handleClientEvent(const IOEvent& event) {
        // 将事件处理提交到线程池
        threadPool_->submit([this, event]() {
            processClientData(event.fd);
        });
    }
    
    void processClientData(int clientFd) {
        std::unique_ptr<Connection> conn;
        {
            std::lock_guard<std::mutex> lock(connectionsMutex_);
            auto it = connections_.find(clientFd);
            if (it == connections_.end()) return;
            conn = std::move(it->second);
            connections_.erase(it);
        }
        
        char buffer[8192];
        std::string allData;
        
        // ET模式需要循环读取
        while (true) {
            ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
            
            if (bytesRead > 0) {
                allData.append(buffer, bytesRead);
                stats_.totalBytes += bytesRead;
            } else if (bytesRead == 0) {
                // 连接关闭
                multiplexer_->removeFd(clientFd);
                close(clientFd);
                stats_.activeConnections--;
                return;
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;  // 数据读完
                } else {
                    // 错误
                    multiplexer_->removeFd(clientFd);
                    close(clientFd);
                    stats_.activeConnections--;
                    stats_.errors++;
                    return;
                }
            }
        }
        
        if (!allData.empty()) {
            stats_.totalMessages++;
            
            // 简单回显
            std::string response = "Echo: " + allData;
            send(clientFd, response.c_str(), response.size(), MSG_NOSIGNAL);
        }
        
        // 更新连接活动时间并放回映射
        conn->lastActivity = std::chrono::steady_clock::now();
        {
            std::lock_guard<std::mutex> lock(connectionsMutex_);
            connections_[clientFd] = std::move(conn);
        }
    }
};

// 模式2: 多Epoll实例模式 (多Reactor模式)
class MultiEpollReactor {
private:
    struct ReactorThread {
        std::unique_ptr<EpollMultiplexer> multiplexer;
        std::thread thread;
        std::atomic<bool> running{false};
        ServerStats stats;
        std::unordered_map<int, std::unique_ptr<Connection>> connections;
        std::mutex connectionsMutex;
        
        ReactorThread() {
            multiplexer = std::make_unique<EpollMultiplexer>(16384, EpollTriggerMode::EdgeTriggered);
        }
    };
    
    std::vector<std::unique_ptr<ReactorThread>> reactors_;
    std::atomic<int> nextReactor_{0};
    int serverFd_;
    std::atomic<bool> running_{false};
    ServerStats globalStats_;
    
public:
    MultiEpollReactor(int reactorCount = 0) {
        if (reactorCount <= 0) {
            reactorCount = std::thread::hardware_concurrency();
        }
        
        std::cout << "多Reactor模式初始化: " << reactorCount << "个Reactor线程\n";
        
        for (int i = 0; i < reactorCount; i++) {
            reactors_.push_back(std::make_unique<ReactorThread>());
        }
    }
    
    void start(int port) {
        serverFd_ = createServerSocket(port);
        if (serverFd_ < 0) {
            throw std::runtime_error("创建服务器socket失败");
        }
        
        std::cout << "多Reactor服务器启动在端口 " << port << "\n";
        
        // 启动所有reactor线程
        for (size_t i = 0; i < reactors_.size(); i++) {
            auto& reactor = reactors_[i];
            reactor->running = true;
            
            reactor->multiplexer->setEventCallback([&reactor](const IOEvent& event) {
                reactor->stats.totalMessages++;
                
                // 直接在reactor线程中处理（避免线程池开销）
                char buffer[8192];
                std::string allData;
                
                while (true) {
                    ssize_t bytesRead = recv(event.fd, buffer, sizeof(buffer), 0);
                    
                    if (bytesRead > 0) {
                        allData.append(buffer, bytesRead);
                        reactor->stats.totalBytes += bytesRead;
                    } else if (bytesRead == 0) {
                        reactor->multiplexer->removeFd(event.fd);
                        close(event.fd);
                        reactor->stats.activeConnections--;
                        return;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            reactor->multiplexer->removeFd(event.fd);
                            close(event.fd);
                            reactor->stats.activeConnections--;
                            reactor->stats.errors++;
                            return;
                        }
                    }
                }
                
                if (!allData.empty()) {
                    std::string response = "MultiReactor: " + allData;
                    send(event.fd, response.c_str(), response.size(), MSG_NOSIGNAL);
                }
            });
            
            reactor->thread = std::thread([&reactor]() {
                reactor->multiplexer->run();
            });
        }
        
        running_ = true;
        
        // 统计线程
        std::thread([this]() {
            while (running_) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                
                // 汇总所有reactor的统计
                globalStats_ = ServerStats{};
                for (const auto& reactor : reactors_) {
                    globalStats_.totalConnections += reactor->stats.totalConnections.load();
                    globalStats_.activeConnections += reactor->stats.activeConnections.load();
                    globalStats_.totalMessages += reactor->stats.totalMessages.load();
                    globalStats_.totalBytes += reactor->stats.totalBytes.load();
                    globalStats_.errors += reactor->stats.errors.load();
                }
                
                std::cout << "=== 多Reactor统计 ===\n";
                globalStats_.printStats();
                std::cout << "Reactor线程数: " << reactors_.size() << "\n\n";
            }
        }).detach();
        
        // 主线程处理accept
        acceptLoop();
    }
    
    void stop() {
        running_ = false;
        for (auto& reactor : reactors_) {
            reactor->running = false;
            reactor->multiplexer->stop();
            if (reactor->thread.joinable()) {
                reactor->thread.join();
            }
        }
        if (serverFd_ >= 0) {
            close(serverFd_);
        }
    }
    
private:
    void acceptLoop() {
        while (running_) {
            sockaddr_in clientAddr{};
            socklen_t addrLen = sizeof(clientAddr);
            int clientFd = accept(serverFd_, (sockaddr*)&clientAddr, &addrLen);
            
            if (clientFd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    continue;
                } else if (errno == EINTR) {
                    continue;
                } else {
                    globalStats_.errors++;
                    continue;
                }
            }
            
            if (!setNonBlocking(clientFd)) {
                close(clientFd);
                globalStats_.errors++;
                continue;
            }
            
            // 负载均衡到不同的reactor
            int reactorIndex = nextReactor_++ % reactors_.size();
            auto& reactor = reactors_[reactorIndex];
            
            if (!reactor->multiplexer->addFd(clientFd, static_cast<uint32_t>(IOEventType::Read))) {
                close(clientFd);
                globalStats_.errors++;
                continue;
            }
            
            reactor->stats.totalConnections++;
            reactor->stats.activeConnections++;
        }
    }
};

// 模式3: Proactor模式模拟 (异步IO模拟)
class ProactorSimulator {
private:
    std::unique_ptr<EpollMultiplexer> multiplexer_;
    std::unique_ptr<IThreadPool> ioThreadPool_;      // IO处理线程池
    std::unique_ptr<IThreadPool> businessThreadPool_; // 业务逻辑线程池
    ServerStats stats_;
    std::atomic<bool> running_{false};
    
    struct AsyncOperation {
        int fd;
        std::string data;
        std::function<void(const std::string&)> callback;
    };
    
public:
    ProactorSimulator() {
        multiplexer_ = std::make_unique<EpollMultiplexer>(32768, EpollTriggerMode::EdgeTriggered);
        
        // IO线程池 - 负责数据读写
        ThreadPoolConfig ioConfig;
        ioConfig.threadCount = std::thread::hardware_concurrency();
        ioConfig.maxQueueSize = 50000;
        ioConfig.rejectionPolicy = RejectionPolicy::Block;
        ioThreadPool_ = ThreadPoolFactory::create(ThreadPoolType::Fixed, ioConfig);
        
        // 业务线程池 - 负责业务逻辑处理
        ThreadPoolConfig businessConfig;
        businessConfig.threadCount = std::thread::hardware_concurrency() * 2;
        businessConfig.maxQueueSize = 100000;
        businessConfig.rejectionPolicy = RejectionPolicy::Block;
        businessThreadPool_ = ThreadPoolFactory::create(ThreadPoolType::Fixed, businessConfig);
        
        std::cout << "Proactor模式初始化: IO线程=" << ioConfig.threadCount 
                  << ", 业务线程=" << businessConfig.threadCount << "\n";
    }
    
    void start(int port) {
        int serverFd = createServerSocket(port);
        if (serverFd < 0) {
            throw std::runtime_error("创建服务器socket失败");
        }
        
        std::cout << "Proactor服务器启动在端口 " << port << "\n";
        
        multiplexer_->addFd(serverFd, static_cast<uint32_t>(IOEventType::Read));
        
        multiplexer_->setEventCallback([this, serverFd](const IOEvent& event) {
            if (event.fd == serverFd) {
                handleAccept(serverFd);
            } else {
                // 异步读取操作
                asyncRead(event.fd, [this](int fd, const std::string& data) {
                    // 业务逻辑处理
                    businessThreadPool_->submit([this, fd, data]() {
                        processBusinessLogic(fd, data);
                    });
                });
            }
        });
        
        running_ = true;
        
        // 统计线程
        std::thread([this]() {
            while (running_) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                std::cout << "=== Proactor统计 ===\n";
                stats_.printStats();
                std::cout << "IO线程池: " << ioThreadPool_->getStats().activeThreads 
                         << "/" << ioThreadPool_->getStats().totalThreads << "\n";
                std::cout << "业务线程池: " << businessThreadPool_->getStats().activeThreads 
                         << "/" << businessThreadPool_->getStats().totalThreads << "\n\n";
            }
        }).detach();
        
        multiplexer_->run();
        close(serverFd);
    }
    
    void stop() {
        running_ = false;
        multiplexer_->stop();
    }
    
private:
    void handleAccept(int serverFd) {
        while (true) {
            sockaddr_in clientAddr{};
            socklen_t addrLen = sizeof(clientAddr);
            int clientFd = accept(serverFd, (sockaddr*)&clientAddr, &addrLen);
            
            if (clientFd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                stats_.errors++;
                break;
            }
            
            if (!setNonBlocking(clientFd)) {
                close(clientFd);
                stats_.errors++;
                continue;
            }
            
            if (!multiplexer_->addFd(clientFd, static_cast<uint32_t>(IOEventType::Read))) {
                close(clientFd);
                stats_.errors++;
                continue;
            }
            
            stats_.totalConnections++;
            stats_.activeConnections++;
        }
    }
    
    void asyncRead(int fd, std::function<void(int, const std::string&)> callback) {
        // 在IO线程池中执行读取操作
        ioThreadPool_->submit([this, fd, callback]() {
            char buffer[8192];
            std::string allData;
            
            while (true) {
                ssize_t bytesRead = recv(fd, buffer, sizeof(buffer), 0);
                
                if (bytesRead > 0) {
                    allData.append(buffer, bytesRead);
                    stats_.totalBytes += bytesRead;
                } else if (bytesRead == 0) {
                    multiplexer_->removeFd(fd);
                    close(fd);
                    stats_.activeConnections--;
                    return;
                } else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break;
                    } else {
                        multiplexer_->removeFd(fd);
                        close(fd);
                        stats_.activeConnections--;
                        stats_.errors++;
                        return;
                    }
                }
            }
            
            if (!allData.empty()) {
                stats_.totalMessages++;
                callback(fd, allData);
            }
        });
    }
    
    void processBusinessLogic(int fd, const std::string& data) {
        // 模拟业务逻辑处理
        std::string response = "Proactor: " + data;
        
        // 异步写入
        asyncWrite(fd, response);
    }
    
    void asyncWrite(int fd, const std::string& data) {
        ioThreadPool_->submit([fd, data]() {
            send(fd, data.c_str(), data.size(), MSG_NOSIGNAL);
        });
    }
};

void printUsage() {
    std::cout << "\n=== 百万并发服务器演示 ===\n";
    std::cout << "支持的架构模式：\n";
    std::cout << "1. 单Epoll+线程池 (Reactor模式)\n";
    std::cout << "   - 特点：一个epoll实例，事件处理使用线程池\n";
    std::cout << "   - 优点：实现简单，适合中等并发\n";
    std::cout << "   - 缺点：epoll可能成为瓶颈\n\n";
    
    std::cout << "2. 多Epoll实例 (多Reactor模式)\n";
    std::cout << "   - 特点：多个epoll实例，每个运行在独立线程\n";
    std::cout << "   - 优点：无锁设计，扩展性好，适合高并发\n";
    std::cout << "   - 缺点：负载均衡需要考虑\n\n";
    
    std::cout << "3. Proactor模式模拟\n";
    std::cout << "   - 特点：分离IO操作和业务逻辑\n";
    std::cout << "   - 优点：更好的分层，业务逻辑不阻塞IO\n";
    std::cout << "   - 缺点：实现复杂，线程切换开销\n\n";
    
    std::cout << "💡 性能优化建议：\n";
    std::cout << "• 使用ET边缘触发模式减少事件数量\n";
    std::cout << "• 设置合适的系统限制 (ulimit -n 1048576)\n";
    std::cout << "• 使用SO_REUSEPORT实现负载均衡\n";
    std::cout << "• 考虑CPU亲和性绑定\n";
    std::cout << "• 使用内存池减少内存分配\n";
    std::cout << "• 批量处理事件减少系统调用\n\n";
}

void runBenchmark(int mode, int port, int duration) {
    std::cout << "运行模式 " << mode << " 基准测试 " << duration << " 秒...\n";
    
    try {
        if (mode == 1) {
            SingleEpollReactor server;
            std::thread serverThread([&server, port]() {
                server.start(port);
            });
            
            std::this_thread::sleep_for(std::chrono::seconds(duration));
            server.stop();
            serverThread.join();
            
        } else if (mode == 2) {
            MultiEpollReactor server;
            std::thread serverThread([&server, port]() {
                server.start(port);
            });
            
            std::this_thread::sleep_for(std::chrono::seconds(duration));
            server.stop();
            serverThread.join();
            
        } else if (mode == 3) {
            ProactorSimulator server;
            std::thread serverThread([&server, port]() {
                server.start(port);
            });
            
            std::this_thread::sleep_for(std::chrono::seconds(duration));
            server.stop();
            serverThread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "服务器错误: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "百万并发服务器演示程序\n";
    std::cout << "========================\n";
    
    setupSystemLimits();
    printUsage();
    
    int mode = 1;      // 默认模式
    int port = 8080;   // 默认端口
    int duration = 60; // 默认运行60秒
    
    // 解析命令行参数
    if (argc >= 2) mode = std::atoi(argv[1]);
    if (argc >= 3) port = std::atoi(argv[2]);
    if (argc >= 4) duration = std::atoi(argv[3]);
    
    if (mode < 1 || mode > 3) {
        std::cerr << "无效的模式，请选择 1-3\n";
        std::cout << "用法: " << argv[0] << " [模式] [端口] [持续时间]\n";
        std::cout << "示例: " << argv[0] << " 2 8080 120\n";
        return 1;
    }
    
    std::cout << "🚀 启动服务器...\n";
    std::cout << "模式: " << mode << ", 端口: " << port << ", 持续时间: " << duration << "秒\n";
    std::cout << "\n测试命令示例：\n";
    std::cout << "# 简单连接测试\n";
    std::cout << "echo 'hello' | nc localhost " << port << "\n\n";
    std::cout << "# 压力测试 (需要安装wrk)\n";
    std::cout << "wrk -t12 -c1000 -d30s --timeout 2s http://localhost:" << port << "/\n\n";
    std::cout << "# 大量连接测试\n";
    std::cout << "for i in {1..1000}; do (echo 'test' | nc localhost " << port << " &); done\n\n";
    
    runBenchmark(mode, port, duration);
    
    std::cout << "\n✅ 演示完成！\n";
    return 0;
}

#else
#include <iostream>

int main() {
    std::cout << "此演示程序仅在Linux系统上可用（需要epoll支持）" << std::endl;
    return 0;
}
#endif 