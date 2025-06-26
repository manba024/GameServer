#include "../include/rpc_client.h"
#include "../include/rpc_server.h"
#include "../include/transport.h"
#include "../include/serializer.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace rpc;

// 计算器服务类
class CalculatorService {
public:
    int add(int a, int b) {
        std::cout << "服务器: 执行加法 " << a << " + " << b << std::endl;
        return a + b;
    }
    
    int subtract(int a, int b) {
        std::cout << "服务器: 执行减法 " << a << " - " << b << std::endl;
        return a - b;
    }
    
    int multiply(int a, int b) {
        std::cout << "服务器: 执行乘法 " << a << " * " << b << std::endl;
        return a * b;
    }
    
    double divide(int a, int b) {
        std::cout << "服务器: 执行除法 " << a << " / " << b << std::endl;
        if (b == 0) {
            throw std::invalid_argument("除数不能为零");
        }
        return static_cast<double>(a) / b;
    }
    
    std::string getInfo() {
        return "RPC计算器服务 v1.0";
    }
};

// 服务器函数
void runServer() {
    std::cout << "=== 启动RPC服务器 ===" << std::endl;
    
    // 创建服务器
    RpcServer server(ProtocolType::TCP, SerializationType::JSON);
    
    // 创建计算器服务实例
    auto calculator = std::make_shared<CalculatorService>();
    
    // 创建服务注册器
    ServiceRegistrar<CalculatorService> registrar(server, calculator);
    
    // 注册方法
    registrar.registerMethod("add", &CalculatorService::add);
    registrar.registerMethod("subtract", &CalculatorService::subtract);
    registrar.registerMethod("multiply", &CalculatorService::multiply);
    registrar.registerMethod("divide", &CalculatorService::divide);
    registrar.registerMethod("getInfo", &CalculatorService::getInfo);
    
    // 设置错误处理器
    server.setErrorHandler([](const std::string& error, ErrorCode code) {
        std::cout << "服务器错误: " << error << " (代码: " << static_cast<int>(code) << ")" << std::endl;
    });
    
    // 设置线程池大小
    server.setThreadPoolSize(4);
    
    // 启动服务器
    ServiceEndpoint endpoint("127.0.0.1", 8080);
    if (server.start(endpoint)) {
        std::cout << "服务器启动成功，监听 " << endpoint.toString() << std::endl;
        
        // 等待客户端连接
        while (server.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            // 显示统计信息
            const auto& stats = server.getStatistics();
            if (stats.total_requests > 0) {
                std::cout << "统计信息 - 总请求: " << stats.total_requests 
                         << ", 成功: " << stats.successful_requests
                         << ", 失败: " << stats.failed_requests
                         << ", 活跃连接: " << stats.active_connections << std::endl;
            }
        }
    } else {
        std::cout << "服务器启动失败!" << std::endl;
    }
}

// 客户端函数
void runClient() {
    std::cout << "=== 启动RPC客户端 ===" << std::endl;
    
    // 等待服务器启动
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 创建客户端
    RpcClient client(ProtocolType::TCP, SerializationType::JSON);
    
    // 设置连接回调
    client.setConnectionCallback([](bool connected, const std::string& endpoint) {
        std::cout << "连接状态变化: " << (connected ? "已连接" : "已断开") 
                 << " - " << endpoint << std::endl;
    });
    
    // 连接到服务器
    ServiceEndpoint endpoint("127.0.0.1", 8080);
    if (!client.connect(endpoint)) {
        std::cout << "连接服务器失败!" << std::endl;
        return;
    }
    
    std::cout << "客户端连接成功!" << std::endl;
    
    try {
        // 测试获取服务信息
        std::cout << "\n--- 测试服务信息 ---" << std::endl;
        auto response1 = client.call("getInfo");
        if (response1.isSuccess()) {
            #if HAS_STD_ANY
            auto info = std::any_cast<std::string>(response1.result);
            #else
            auto info = response1.result.cast<std::string>();
            #endif
            std::cout << "服务信息: " << info << std::endl;
        } else {
            std::cout << "获取服务信息失败: " << response1.error_message << std::endl;
        }
        
        // 测试加法
        std::cout << "\n--- 测试加法 ---" << std::endl;
        std::vector<AnyValue> add_params = {AnyValue(10), AnyValue(20)};
        auto response2 = client.call("add", add_params);
        if (response2.isSuccess()) {
            #if HAS_STD_ANY
            auto result = std::any_cast<int>(response2.result);
            #else
            auto result = response2.result.cast<int>();
            #endif
            std::cout << "10 + 20 = " << result << std::endl;
        } else {
            std::cout << "加法调用失败: " << response2.error_message << std::endl;
        }
        
        // 测试减法
        std::cout << "\n--- 测试减法 ---" << std::endl;
        std::vector<AnyValue> sub_params = {AnyValue(50), AnyValue(30)};
        auto response3 = client.call("subtract", sub_params);
        if (response3.isSuccess()) {
            #if HAS_STD_ANY
            auto result = std::any_cast<int>(response3.result);
            #else
            auto result = response3.result.cast<int>();
            #endif
            std::cout << "50 - 30 = " << result << std::endl;
        } else {
            std::cout << "减法调用失败: " << response3.error_message << std::endl;
        }
        
        // 测试乘法
        std::cout << "\n--- 测试乘法 ---" << std::endl;
        std::vector<AnyValue> mul_params = {AnyValue(6), AnyValue(7)};
        auto response4 = client.call("multiply", mul_params);
        if (response4.isSuccess()) {
            #if HAS_STD_ANY
            auto result = std::any_cast<int>(response4.result);
            #else
            auto result = response4.result.cast<int>();
            #endif
            std::cout << "6 * 7 = " << result << std::endl;
        } else {
            std::cout << "乘法调用失败: " << response4.error_message << std::endl;
        }
        
        // 测试除法
        std::cout << "\n--- 测试除法 ---" << std::endl;
        std::vector<AnyValue> div_params = {AnyValue(100), AnyValue(4)};
        auto response5 = client.call("divide", div_params);
        if (response5.isSuccess()) {
            #if HAS_STD_ANY
            auto result = std::any_cast<double>(response5.result);
            #else
            auto result = response5.result.cast<double>();
            #endif
            std::cout << "100 / 4 = " << result << std::endl;
        } else {
            std::cout << "除法调用失败: " << response5.error_message << std::endl;
        }
        
        // 测试异步调用
        std::cout << "\n--- 测试异步调用 ---" << std::endl;
        std::vector<AnyValue> async_params = {AnyValue(15), AnyValue(25)};
        auto future = client.callAsync("add", async_params);
        
        std::cout << "异步调用已发送，等待结果..." << std::endl;
        auto async_response = future.get();
        if (async_response.isSuccess()) {
            #if HAS_STD_ANY
            auto result = std::any_cast<int>(async_response.result);
            #else
            auto result = async_response.result.cast<int>();
            #endif
            std::cout << "异步结果: 15 + 25 = " << result << std::endl;
        } else {
            std::cout << "异步调用失败: " << async_response.error_message << std::endl;
        }
        
        // 测试错误情况
        std::cout << "\n--- 测试错误情况 ---" << std::endl;
        std::vector<AnyValue> error_params = {AnyValue(10), AnyValue(0)};
        auto error_response = client.call("divide", error_params);
        if (!error_response.isSuccess()) {
            std::cout << "预期的错误: " << error_response.error_message << std::endl;
        }
        
        // 显示客户端统计信息
        std::cout << "\n--- 客户端统计信息 ---" << std::endl;
        const auto& stats = client.getStatistics();
        std::cout << "总请求: " << stats.total_requests 
                 << ", 成功: " << stats.successful_requests
                 << ", 失败: " << stats.failed_requests
                 << ", 超时: " << stats.timeout_requests
                 << ", 发送字节: " << stats.bytes_sent
                 << ", 接收字节: " << stats.bytes_received << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "客户端异常: " << e.what() << std::endl;
    }
    
    // 断开连接
    client.disconnect();
    std::cout << "客户端已断开连接" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "RPC框架演示程序" << std::endl;
    std::cout << "支持的协议: TCP, HTTP, UDP" << std::endl;
    std::cout << "支持的序列化: JSON, Binary, MessagePack" << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (argc > 1 && std::string(argv[1]) == "server") {
        // 只运行服务器
        runServer();
    } else if (argc > 1 && std::string(argv[1]) == "client") {
        // 只运行客户端
        runClient();
    } else {
        // 同时运行服务器和客户端
        std::thread server_thread(runServer);
        std::thread client_thread(runClient);
        
        client_thread.join();
        
        // 等待一段时间后停止服务器
        std::this_thread::sleep_for(std::chrono::seconds(2));
        server_thread.detach(); // 让服务器线程自然结束
    }
    
    return 0;
}

// 编译说明:
// g++ -std=c++14 -pthread -I../include calculator_demo.cpp ../src/*.cpp -o calculator_demo
//
// 运行说明:
// ./calculator_demo          # 同时运行服务器和客户端
// ./calculator_demo server   # 只运行服务器
// ./calculator_demo client   # 只运行客户端 