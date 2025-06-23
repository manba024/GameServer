#include "../include/multiplexer_factory.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

using namespace IOMultiplexing;

// 演示数据结构
struct ConnectionData {
    int clientFd;
    std::string name;
    
    ConnectionData(int fd, const std::string& n) : clientFd(fd), name(n) {}
};

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
    
    // 设置地址重用
    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 绑定地址
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "绑定地址失败" << std::endl;
        close(serverFd);
        return -1;
    }
    
    // 开始监听
    if (listen(serverFd, 10) < 0) {
        std::cerr << "监听失败" << std::endl;
        close(serverFd);
        return -1;
    }
    
    // 设置非阻塞
    if (!setNonBlocking(serverFd)) {
        std::cerr << "设置非阻塞失败" << std::endl;
        close(serverFd);
        return -1;
    }
    
    return serverFd;
}

void demonstrateBasicUsage() {
    std::cout << "=== 基础使用演示 ===\n";
    
    // 显示支持的IO复用器类型
    std::cout << "支持的IO复用器类型：\n";
    auto supportedTypes = MultiplexerFactory::getSupportedTypes();
    for (auto type : supportedTypes) {
        std::cout << "- " << MultiplexerFactory::getTypeName(type) << std::endl;
    }
    
    // 创建最佳的IO复用器
    auto multiplexer = createBestMultiplexer();
    if (!multiplexer) {
        std::cout << "创建IO复用器失败！\n";
        return;
    }
    
    std::cout << "\n使用 " << multiplexer->getTypeName() << " IO复用器\n";
    std::cout << "最大支持文件描述符数: " << multiplexer->getMaxFdCount() << "\n";
    
    // 创建服务器socket
    int serverFd = createServerSocket(8080);
    if (serverFd < 0) {
        return;
    }
    
    std::cout << "服务器启动在端口 8080\n";
    std::cout << "等待连接...\n";
    
    // 添加服务器socket到IO复用器
    multiplexer->addFd(serverFd, static_cast<uint32_t>(IOEventType::Read));
    
    // 设置事件回调
    multiplexer->setEventCallback([&](const IOEvent& event) {
        if (event.fd == serverFd) {
            // 处理新连接
            sockaddr_in clientAddr{};
            socklen_t addrLen = sizeof(clientAddr);
            int clientFd = accept(serverFd, (sockaddr*)&clientAddr, &addrLen);
            
            if (clientFd >= 0) {
                setNonBlocking(clientFd);
                
                // 为客户端创建用户数据
                ConnectionData* connData = new ConnectionData(clientFd, "客户端" + std::to_string(clientFd));
                
                // 添加客户端socket到IO复用器
                multiplexer->addFd(clientFd, 
                    static_cast<uint32_t>(IOEventType::Read) | 
                    static_cast<uint32_t>(IOEventType::Error),
                    connData);
                
                std::cout << "新连接: " << connData->name << " (fd=" << clientFd << ")\n";
                
                // 发送欢迎消息
                std::string welcome = "欢迎连接到IO复用演示服务器!\n";
                send(clientFd, welcome.c_str(), welcome.size(), 0);
            }
        } else {
            // 处理客户端事件
            ConnectionData* connData = static_cast<ConnectionData*>(event.userData);
            
            if (event.events & static_cast<uint32_t>(IOEventType::Read)) {
                char buffer[1024];
                ssize_t bytesRead = recv(event.fd, buffer, sizeof(buffer) - 1, 0);
                
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    std::cout << connData->name << " 发送: " << buffer;
                    
                    // 回显消息
                    std::string response = "回显: " + std::string(buffer);
                    send(event.fd, response.c_str(), response.size(), 0);
                } else if (bytesRead == 0) {
                    // 连接关闭
                    std::cout << connData->name << " 断开连接\n";
                    multiplexer->removeFd(event.fd);
                    close(event.fd);
                    delete connData;
                }
            }
            
            if (event.events & static_cast<uint32_t>(IOEventType::Error)) {
                std::cout << connData->name << " 发生错误\n";
                multiplexer->removeFd(event.fd);
                close(event.fd);
                delete connData;
            }
        }
    });
    
    // 运行5秒钟
    std::cout << "服务器将运行5秒钟...\n";
    std::cout << "你可以使用 'telnet localhost 8080' 来连接\n";
    
    std::thread([&multiplexer]() {
        multiplexer->run();
    }).detach();
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    multiplexer->stop();
    
    close(serverFd);
    std::cout << "服务器停止\n";
}

void demonstrateMultipleTypes() {
    std::cout << "\n=== 多种IO复用器比较演示 ===\n";
    
    std::vector<MultiplexerType> typesToTest = {
        MultiplexerType::Select,
        MultiplexerType::Poll
    };
    
#ifdef __linux__
    typesToTest.push_back(MultiplexerType::Epoll);
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    typesToTest.push_back(MultiplexerType::Kqueue);
#endif
    
    for (auto type : typesToTest) {
        if (MultiplexerFactory::isSupported(type)) {
            auto multiplexer = createMultiplexer(type);
            if (multiplexer) {
                std::cout << "\n" << multiplexer->getTypeName() << " IO复用器:\n";
                std::cout << "  最大文件描述符数: " << multiplexer->getMaxFdCount() << "\n";
                std::cout << "  当前文件描述符数: " << multiplexer->getFdCount() << "\n";
                
                // 简单测试：添加和删除文件描述符
                multiplexer->addFd(0, static_cast<uint32_t>(IOEventType::Read)); // stdin
                std::cout << "  添加stdin后: " << multiplexer->getFdCount() << " 个fd\n";
                
                multiplexer->removeFd(0);
                std::cout << "  删除stdin后: " << multiplexer->getFdCount() << " 个fd\n";
            }
        } else {
            std::cout << "\n" << MultiplexerFactory::getTypeName(type) << " 在当前平台不支持\n";
        }
    }
}

void demonstrateRecommendations() {
    std::cout << "\n=== 推荐的IO复用器（按性能排序） ===\n";
    
    auto recommended = MultiplexerFactory::getRecommendedTypes();
    for (size_t i = 0; i < recommended.size(); i++) {
        auto type = recommended[i];
        std::cout << (i + 1) << ". " << MultiplexerFactory::getTypeName(type);
        
        if (MultiplexerFactory::isSupported(type)) {
            std::cout << " ✓ (支持)";
        } else {
            std::cout << " ✗ (不支持)";
        }
        std::cout << "\n";
    }
    
    std::cout << "\n当前平台最佳选择: ";
    auto best = createBestMultiplexer();
    if (best) {
        std::cout << best->getTypeName() << "\n";
    }
}

int main() {
    std::cout << "IO复用框架演示程序\n";
    std::cout << "===================\n";
    
    try {
        // 多种IO复用器比较
        demonstrateMultipleTypes();
        
        // 推荐演示
        demonstrateRecommendations();
        
        // 基础使用演示（网络服务器）
        demonstrateBasicUsage();
        
        std::cout << "\n=== 演示完成 ===\n";
        std::cout << "了解更多:\n";
        std::cout << "- Select: 最古老、兼容性最好，但性能较差，有fd数量限制\n";
        std::cout << "- Poll: 改进了select的一些问题，无fd数量限制\n";
        std::cout << "- Epoll: Linux特有，高性能，适合大量连接\n";
        std::cout << "- Kqueue: BSD/macOS特有，高性能，功能丰富\n";
        
    } catch (const std::exception& e) {
        std::cerr << "演示过程中发生错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 