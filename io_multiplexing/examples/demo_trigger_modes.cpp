#ifdef __linux__

#include "../include/multiplexer_factory.h"
#include "../include/epoll_multiplexer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

using namespace IOMultiplexing;

// 设置非阻塞模式
bool setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

// 创建服务器socket
int createServerSocket(int port) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        std::cerr << "创建socket失败" << std::endl;
        return -1;
    }
    
    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "绑定地址失败" << std::endl;
        close(serverFd);
        return -1;
    }
    
    if (listen(serverFd, 10) < 0) {
        std::cerr << "监听失败" << std::endl;
        close(serverFd);
        return -1;
    }
    
    if (!setNonBlocking(serverFd)) {
        std::cerr << "设置非阻塞失败" << std::endl;
        close(serverFd);
        return -1;
    }
    
    return serverFd;
}

void demonstrateLevelTriggered() {
    std::cout << "\n=== LT（水平触发）模式演示 ===\n";
    std::cout << "LT模式特点：只要缓冲区有数据可读，就会持续触发事件\n";
    
    // 创建LT模式的epoll复用器
    EpollMultiplexer multiplexer(1024, EpollTriggerMode::LevelTriggered);
    
    int serverFd = createServerSocket(8081);
    if (serverFd < 0) return;
    
    std::cout << "LT模式服务器启动在端口 8081\n";
    std::cout << "使用 'echo \"hello world\" | nc localhost 8081' 来测试\n";
    
    // 添加服务器socket（LT模式）
    multiplexer.addFd(serverFd, static_cast<uint32_t>(IOEventType::Read));
    
    int eventCount = 0;
    bool partialRead = false;  // 模拟部分读取
    
    multiplexer.setEventCallback([&](const IOEvent& event) {
        eventCount++;
        std::cout << "LT事件 #" << eventCount << ": fd=" << event.fd << std::endl;
        
        if (event.fd == serverFd) {
            // 新连接
            sockaddr_in clientAddr{};
            socklen_t addrLen = sizeof(clientAddr);
            int clientFd = accept(serverFd, (sockaddr*)&clientAddr, &addrLen);
            
            if (clientFd >= 0) {
                setNonBlocking(clientFd);
                std::cout << "LT: 新连接 fd=" << clientFd << std::endl;
                
                // 添加客户端socket（LT模式）
                multiplexer.addFd(clientFd, static_cast<uint32_t>(IOEventType::Read));
            }
        } else {
            // 客户端数据
            if (event.events & static_cast<uint32_t>(IOEventType::Read)) {
                char buffer[1024];
                
                if (!partialRead) {
                    // 第一次只读取一小部分数据，模拟部分读取
                    ssize_t bytesRead = recv(event.fd, buffer, 5, 0);  // 只读5字节
                    if (bytesRead > 0) {
                        buffer[bytesRead] = '\0';
                        std::cout << "LT: 部分读取(" << bytesRead << "字节): \"" << buffer << "\"\n";
                        std::cout << "LT: 缓冲区还有数据，LT模式会继续触发事件\n";
                        partialRead = true;
                    }
                } else {
                    // 第二次读取剩余数据
                    ssize_t bytesRead = recv(event.fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytesRead > 0) {
                        buffer[bytesRead] = '\0';
                        std::cout << "LT: 读取剩余数据(" << bytesRead << "字节): \"" << buffer << "\"\n";
                        
                        // 回显
                        std::string response = "LT回显: " + std::string(buffer);
                        send(event.fd, response.c_str(), response.size(), 0);
                    } else if (bytesRead == 0) {
                        std::cout << "LT: 连接关闭\n";
                        multiplexer.removeFd(event.fd);
                        close(event.fd);
                    }
                    partialRead = false;
                }
            }
        }
    });
    
    // 运行5秒
    std::thread([&multiplexer]() {
        multiplexer.run();
    }).detach();
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    multiplexer.stop();
    close(serverFd);
    
    std::cout << "LT模式总事件数: " << eventCount << std::endl;
}

void demonstrateEdgeTriggered() {
    std::cout << "\n=== ET（边缘触发）模式演示 ===\n";
    std::cout << "ET模式特点：只在状态变化时触发一次事件，需要一次性读完所有数据\n";
    
    // 创建ET模式的epoll复用器
    EpollMultiplexer multiplexer(1024, EpollTriggerMode::EdgeTriggered);
    
    int serverFd = createServerSocket(8082);
    if (serverFd < 0) return;
    
    std::cout << "ET模式服务器启动在端口 8082\n";
    std::cout << "使用 'echo \"hello world\" | nc localhost 8082' 来测试\n";
    
    // 添加服务器socket（ET模式）
    multiplexer.addFd(serverFd, static_cast<uint32_t>(IOEventType::Read));
    
    int eventCount = 0;
    
    multiplexer.setEventCallback([&](const IOEvent& event) {
        eventCount++;
        std::cout << "ET事件 #" << eventCount << ": fd=" << event.fd << std::endl;
        
        if (event.fd == serverFd) {
            // 新连接 - ET模式需要循环accept直到EAGAIN
            while (true) {
                sockaddr_in clientAddr{};
                socklen_t addrLen = sizeof(clientAddr);
                int clientFd = accept(serverFd, (sockaddr*)&clientAddr, &addrLen);
                
                if (clientFd >= 0) {
                    setNonBlocking(clientFd);
                    std::cout << "ET: 新连接 fd=" << clientFd << std::endl;
                    
                    // 添加客户端socket（ET模式）
                    multiplexer.addFd(clientFd, static_cast<uint32_t>(IOEventType::Read));
                } else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        std::cout << "ET: accept完成，没有更多连接\n";
                    } else {
                        std::cerr << "ET: accept错误: " << strerror(errno) << std::endl;
                    }
                    break;
                }
            }
        } else {
            // 客户端数据 - ET模式需要循环读取直到EAGAIN
            if (event.events & static_cast<uint32_t>(IOEventType::Read)) {
                std::cout << "ET: 开始读取数据，需要一次性读完\n";
                
                std::string allData;
                char buffer[1024];
                
                while (true) {
                    ssize_t bytesRead = recv(event.fd, buffer, sizeof(buffer) - 1, 0);
                    
                    if (bytesRead > 0) {
                        buffer[bytesRead] = '\0';
                        allData += buffer;
                        std::cout << "ET: 读取数据块(" << bytesRead << "字节): \"" << buffer << "\"\n";
                    } else if (bytesRead == 0) {
                        std::cout << "ET: 连接关闭\n";
                        multiplexer.removeFd(event.fd);
                        close(event.fd);
                        break;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            std::cout << "ET: 数据读取完成，总共读取: \"" << allData << "\"\n";
                            
                            if (!allData.empty()) {
                                // 回显
                                std::string response = "ET回显: " + allData;
                                send(event.fd, response.c_str(), response.size(), 0);
                            }
                        } else {
                            std::cerr << "ET: 读取错误: " << strerror(errno) << std::endl;
                        }
                        break;
                    }
                }
            }
        }
    });
    
    // 运行5秒
    std::thread([&multiplexer]() {
        multiplexer.run();
    }).detach();
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    multiplexer.stop();
    close(serverFd);
    
    std::cout << "ET模式总事件数: " << eventCount << std::endl;
}

void demonstrateMixedModes() {
    std::cout << "\n=== 混合模式演示 ===\n";
    std::cout << "服务器socket使用LT模式，客户端socket使用ET模式\n";
    
    // 创建默认LT模式的epoll复用器
    EpollMultiplexer multiplexer(1024, EpollTriggerMode::LevelTriggered);
    
    int serverFd = createServerSocket(8083);
    if (serverFd < 0) return;
    
    std::cout << "混合模式服务器启动在端口 8083\n";
    
    // 服务器socket使用LT模式
    multiplexer.addFd(serverFd, static_cast<uint32_t>(IOEventType::Read), EpollTriggerMode::LevelTriggered);
    
    int ltEvents = 0, etEvents = 0;
    
    multiplexer.setEventCallback([&](const IOEvent& event) {
        if (event.fd == serverFd) {
            ltEvents++;
            std::cout << "LT事件(服务器): 新连接\n";
            
            sockaddr_in clientAddr{};
            socklen_t addrLen = sizeof(clientAddr);
            int clientFd = accept(serverFd, (sockaddr*)&clientAddr, &addrLen);
            
            if (clientFd >= 0) {
                setNonBlocking(clientFd);
                std::cout << "新连接 fd=" << clientFd << "，使用ET模式\n";
                
                // 客户端socket使用ET模式
                multiplexer.addFd(clientFd, static_cast<uint32_t>(IOEventType::Read), EpollTriggerMode::EdgeTriggered);
            }
        } else {
            etEvents++;
            std::cout << "ET事件(客户端): 数据到达\n";
            
            // ET模式处理
            std::string allData;
            char buffer[1024];
            
            while (true) {
                ssize_t bytesRead = recv(event.fd, buffer, sizeof(buffer) - 1, 0);
                
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    allData += buffer;
                } else if (bytesRead == 0) {
                    std::cout << "ET: 连接关闭\n";
                    multiplexer.removeFd(event.fd);
                    close(event.fd);
                    break;
                } else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        if (!allData.empty()) {
                            std::cout << "ET: 读取完成: \"" << allData << "\"\n";
                            std::string response = "混合模式回显: " + allData;
                            send(event.fd, response.c_str(), response.size(), 0);
                        }
                    }
                    break;
                }
            }
        }
    });
    
    // 运行5秒
    std::thread([&multiplexer]() {
        multiplexer.run();
    }).detach();
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    multiplexer.stop();
    close(serverFd);
    
    std::cout << "混合模式 - LT事件: " << ltEvents << ", ET事件: " << etEvents << std::endl;
}

void printTriggerModeComparison() {
    std::cout << "\n=== LT vs ET 触发模式对比 ===\n";
    std::cout << "┌─────────────┬─────────────────────┬─────────────────────┐\n";
    std::cout << "│    特性     │      LT水平触发     │      ET边缘触发     │\n";
    std::cout << "├─────────────┼─────────────────────┼─────────────────────┤\n";
    std::cout << "│  触发条件   │ 缓冲区有数据就触发  │ 状态变化时才触发    │\n";
    std::cout << "│  事件频率   │      较高           │      较低           │\n";
    std::cout << "│  编程难度   │      简单           │      复杂           │\n";
    std::cout << "│  性能       │      一般           │      更高           │\n";
    std::cout << "│  数据处理   │   可以部分读取      │  必须一次读完       │\n";
    std::cout << "│  适用场景   │   简单应用          │  高并发服务器       │\n";
    std::cout << "└─────────────┴─────────────────────┴─────────────────────┘\n";
    
    std::cout << "\n📝 编程要点：\n";
    std::cout << "• LT模式：类似select/poll，简单易用，适合初学者\n";
    std::cout << "• ET模式：需要循环读取直到EAGAIN，适合高性能场景\n";
    std::cout << "• 混合模式：可以为不同fd设置不同触发模式\n";
    std::cout << "• ET模式必须使用非阻塞IO，否则可能导致程序阻塞\n";
}

int main() {
    std::cout << "Epoll触发模式演示程序\n";
    std::cout << "====================\n";
    
    printTriggerModeComparison();
    
    std::cout << "\n🚀 开始演示...\n";
    std::cout << "请在另一个终端使用以下命令测试：\n";
    std::cout << "echo \"hello world\" | nc localhost <端口>\n";
    
    try {
        // 演示LT模式
        demonstrateLevelTriggered();
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 演示ET模式
        demonstrateEdgeTriggered();
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 演示混合模式
        demonstrateMixedModes();
        
        std::cout << "\n✅ 演示完成！\n";
        std::cout << "\n💡 总结：\n";
        std::cout << "- LT模式会持续触发事件，直到数据被完全读取\n";
        std::cout << "- ET模式只在状态变化时触发一次，需要循环读取\n";
        std::cout << "- 可以为不同的fd设置不同的触发模式\n";
        std::cout << "- 选择合适的触发模式可以优化服务器性能\n";
        
    } catch (const std::exception& e) {
        std::cerr << "演示过程中发生错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

#else
#include <iostream>

int main() {
    std::cout << "此演示程序仅在Linux系统上可用（需要epoll支持）" << std::endl;
    return 0;
}
#endif 