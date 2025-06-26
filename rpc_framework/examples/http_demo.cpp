#include "../include/rpc_client.h"
#include "../include/rpc_server.h"
#include "../include/transport.h"
#include "../include/serializer.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace rpc;

// 简单的Web服务类
class WebService {
public:
    std::string echo(const std::string& message) {
        std::cout << "服务器收到回显请求: " << message << std::endl;
        return "Echo: " + message;
    }
    
    std::string getTime() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        return std::ctime(&time_t);
    }
    
    int fibonacci(int n) {
        std::cout << "服务器计算斐波那契数列: " << n << std::endl;
        if (n <= 1) return n;
        
        int a = 0, b = 1;
        for (int i = 2; i <= n; ++i) {
            int temp = a + b;
            a = b;
            b = temp;
        }
        return b;
    }
};

// HTTP服务器函数
void runHttpServer() {
    std::cout << "=== 启动HTTP RPC服务器 ===" << std::endl;
    
    // 创建HTTP服务器
    RpcServer server(ProtocolType::HTTP, SerializationType::JSON);
    
    // 创建Web服务实例
    auto webService = std::make_shared<WebService>();
    
    // 注册服务方法
    ServiceRegistrar<WebService> registrar(server, webService);
    registrar.registerMethod("echo", &WebService::echo);
    registrar.registerMethod("getTime", &WebService::getTime);
    registrar.registerMethod("fibonacci", &WebService::fibonacci);
    
    // 设置线程池
    server.setThreadPoolSize(2);
    
    // 启动服务器
    ServiceEndpoint endpoint("127.0.0.1", 8081);
    if (server.start(endpoint)) {
        std::cout << "HTTP服务器启动成功，监听 " << endpoint.toString() << std::endl;
        std::cout << "可以通过以下URL访问:" << std::endl;
        std::cout << "  curl -X POST http://127.0.0.1:8081/rpc \\" << std::endl;
        std::cout << "       -H 'Content-Type: application/json' \\" << std::endl;
        std::cout << "       -d '{\"method\":\"echo\",\"params\":[\"Hello World\"]}'" << std::endl;
        
        // 保持服务器运行
        while (server.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            // 显示统计信息
            const auto& stats = server.getStatistics();
            if (stats.total_requests > 0) {
                std::cout << "HTTP统计 - 总请求: " << stats.total_requests 
                         << ", 成功: " << stats.successful_requests
                         << ", 失败: " << stats.failed_requests << std::endl;
            }
        }
    } else {
        std::cout << "HTTP服务器启动失败!" << std::endl;
    }
}

// HTTP客户端函数
void runHttpClient() {
    std::cout << "=== 启动HTTP RPC客户端 ===" << std::endl;
    
    // 等待服务器启动
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 创建HTTP客户端
    RpcClient client(ProtocolType::HTTP, SerializationType::JSON);
    
    // 连接到服务器
    ServiceEndpoint endpoint("127.0.0.1", 8081);
    if (!client.connect(endpoint)) {
        std::cout << "连接HTTP服务器失败!" << std::endl;
        return;
    }
    
    std::cout << "HTTP客户端连接成功!" << std::endl;
    
    try {
        // 测试回显服务
        std::cout << "\n--- 测试回显服务 ---" << std::endl;
        std::vector<AnyValue> echo_params = {AnyValue(std::string("Hello HTTP RPC!"))};
        auto response1 = client.call("echo", echo_params);
        if (response1.isSuccess()) {
            #if HAS_STD_ANY
            auto result = std::any_cast<std::string>(response1.result);
            #else
            auto result = response1.result.cast<std::string>();
            #endif
            std::cout << "回显结果: " << result << std::endl;
        } else {
            std::cout << "回显调用失败: " << response1.error_message << std::endl;
        }
        
        // 测试获取时间
        std::cout << "\n--- 测试获取时间 ---" << std::endl;
        auto response2 = client.call("getTime");
        if (response2.isSuccess()) {
            #if HAS_STD_ANY
            auto result = std::any_cast<std::string>(response2.result);
            #else
            auto result = response2.result.cast<std::string>();
            #endif
            std::cout << "服务器时间: " << result << std::endl;
        } else {
            std::cout << "获取时间失败: " << response2.error_message << std::endl;
        }
        
        // 测试斐波那契数列
        std::cout << "\n--- 测试斐波那契数列 ---" << std::endl;
        std::vector<AnyValue> fib_params = {AnyValue(10)};
        auto response3 = client.call("fibonacci", fib_params);
        if (response3.isSuccess()) {
            #if HAS_STD_ANY
            auto result = std::any_cast<int>(response3.result);
            #else
            auto result = response3.result.cast<int>();
            #endif
            std::cout << "斐波那契(10) = " << result << std::endl;
        } else {
            std::cout << "斐波那契调用失败: " << response3.error_message << std::endl;
        }
        
        // 测试异步调用
        std::cout << "\n--- 测试异步调用 ---" << std::endl;
        std::vector<AnyValue> async_params = {AnyValue(15)};
        auto future = client.callAsync("fibonacci", async_params);
        
        std::cout << "异步调用已发送，等待结果..." << std::endl;
        auto async_response = future.get();
        if (async_response.isSuccess()) {
            #if HAS_STD_ANY
            auto result = std::any_cast<int>(async_response.result);
            #else
            auto result = async_response.result.cast<int>();
            #endif
            std::cout << "异步结果: 斐波那契(15) = " << result << std::endl;
        } else {
            std::cout << "异步调用失败: " << async_response.error_message << std::endl;
        }
        
        // 显示统计信息
        std::cout << "\n--- HTTP客户端统计信息 ---" << std::endl;
        const auto& stats = client.getStatistics();
        std::cout << "总请求: " << stats.total_requests 
                 << ", 成功: " << stats.successful_requests
                 << ", 失败: " << stats.failed_requests << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "HTTP客户端异常: " << e.what() << std::endl;
    }
    
    // 断开连接
    client.disconnect();
    std::cout << "HTTP客户端已断开连接" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "HTTP RPC框架演示程序" << std::endl;
    std::cout << "使用HTTP协议和JSON序列化" << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (argc > 1 && std::string(argv[1]) == "server") {
        // 只运行服务器
        runHttpServer();
    } else if (argc > 1 && std::string(argv[1]) == "client") {
        // 只运行客户端
        runHttpClient();
    } else {
        // 同时运行服务器和客户端
        std::thread server_thread(runHttpServer);
        std::thread client_thread(runHttpClient);
        
        client_thread.join();
        
        // 等待一段时间后停止服务器
        std::this_thread::sleep_for(std::chrono::seconds(2));
        server_thread.detach();
    }
    
    return 0;
}

// 编译说明:
// g++ -std=c++14 -pthread -I../include http_demo.cpp ../src/*.cpp -o http_demo
//
// 运行说明:
// ./http_demo          # 同时运行服务器和客户端
// ./http_demo server   # 只运行HTTP服务器
// ./http_demo client   # 只运行HTTP客户端
//
// 手动测试HTTP API:
// curl -X POST http://127.0.0.1:8081/rpc \
//      -H 'Content-Type: application/json' \
//      -d '{"method":"echo","params":["Hello World"]}' 