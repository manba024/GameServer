#include "../include/rpc_client.h"
#include "../include/rpc_server.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <future>

using namespace rpc;

// 测试服务类
class TestService {
public:
    // 基本数据类型测试
    int testInt(int value) {
        std::cout << "服务器: testInt(" << value << ")" << std::endl;
        return value * 2;
    }
    
    double testDouble(double value) {
        std::cout << "服务器: testDouble(" << value << ")" << std::endl;
        return value * 3.14;
    }
    
    bool testBool(bool value) {
        std::cout << "服务器: testBool(" << (value ? "true" : "false") << ")" << std::endl;
        return !value;
    }
    
    std::string testString(const std::string& value) {
        std::cout << "服务器: testString(\"" << value << "\")" << std::endl;
        return "Hello, " + value + "!";
    }
    
    // 多参数测试
    int testMultiParams(int a, int b, int c) {
        std::cout << "服务器: testMultiParams(" << a << ", " << b << ", " << c << ")" << std::endl;
        return a + b + c;
    }
    
    // 无参数测试
    std::string testNoParams() {
        std::cout << "服务器: testNoParams()" << std::endl;
        return "No parameters test passed!";
    }
    
    // 错误测试
    int testError(int value) {
        std::cout << "服务器: testError(" << value << ")" << std::endl;
        if (value < 0) {
            throw std::invalid_argument("Value must be non-negative");
        }
        return value * value;
    }
};

bool runComprehensiveTest() {
    std::cout << "=== 启动全面测试 ===" << std::endl;
    
    // 创建服务器
    RpcServer server(ProtocolType::TCP, SerializationType::JSON);
    auto service = std::make_shared<TestService>();
    
    // 使用ServiceRegistrar注册所有方法
    ServiceRegistrar<TestService> registrar(server, service);
    registrar.registerMethod("testInt", &TestService::testInt);
    registrar.registerMethod("testDouble", &TestService::testDouble);
    registrar.registerMethod("testBool", &TestService::testBool);
    registrar.registerMethod("testString", &TestService::testString);
    registrar.registerMethod("testMultiParams", &TestService::testMultiParams);
    registrar.registerMethod("testNoParams", &TestService::testNoParams);
    registrar.registerMethod("testError", &TestService::testError);
    
    // 启动服务器
    ServiceEndpoint endpoint("127.0.0.1", 8085);
    if (!server.start(endpoint)) {
        std::cout << "服务器启动失败!" << std::endl;
        return false;
    }
    
    std::cout << "服务器启动成功，监听 " << endpoint.toString() << std::endl;
    
    // 等待服务器启动
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 创建客户端
    RpcClient client(ProtocolType::TCP, SerializationType::JSON);
    if (!client.connect(endpoint)) {
        std::cout << "连接服务器失败!" << std::endl;
        return false;
    }
    
    std::cout << "客户端连接成功!" << std::endl;
    
    bool all_tests_passed = true;
    
    try {
        // 测试1: 整数
        std::cout << "\n--- 测试整数 ---" << std::endl;
        auto response1 = client.call("testInt", {AnyValue(42)});
        if (response1.isSuccess()) {
            int result = response1.result.cast<int>();
            std::cout << "结果: " << result << " (期望: 84)" << std::endl;
            if (result != 84) all_tests_passed = false;
        } else {
            std::cout << "测试失败: " << response1.error_message << std::endl;
            all_tests_passed = false;
        }
        
        // 测试2: 浮点数
        std::cout << "\n--- 测试浮点数 ---" << std::endl;
        auto response2 = client.call("testDouble", {AnyValue(2.0)});
        if (response2.isSuccess()) {
            double result = response2.result.cast<double>();
            std::cout << "结果: " << result << " (期望: 6.28)" << std::endl;
            if (std::abs(result - 6.28) > 0.01) all_tests_passed = false;
        } else {
            std::cout << "测试失败: " << response2.error_message << std::endl;
            all_tests_passed = false;
        }
        
        // 测试3: 布尔值
        std::cout << "\n--- 测试布尔值 ---" << std::endl;
        auto response3 = client.call("testBool", {AnyValue(true)});
        if (response3.isSuccess()) {
            bool result = response3.result.cast<bool>();
            std::cout << "结果: " << (result ? "true" : "false") << " (期望: false)" << std::endl;
            if (result != false) all_tests_passed = false;
        } else {
            std::cout << "测试失败: " << response3.error_message << std::endl;
            all_tests_passed = false;
        }
        
        // 测试4: 字符串
        std::cout << "\n--- 测试字符串 ---" << std::endl;
        auto response4 = client.call("testString", {AnyValue("World")});
        if (response4.isSuccess()) {
            std::string result = response4.result.cast<std::string>();
            std::cout << "结果: \"" << result << "\" (期望: \"Hello, World!\")" << std::endl;
            if (result != "Hello, World!") all_tests_passed = false;
        } else {
            std::cout << "测试失败: " << response4.error_message << std::endl;
            all_tests_passed = false;
        }
        
        // 测试5: 多参数
        std::cout << "\n--- 测试多参数 ---" << std::endl;
        auto response5 = client.call("testMultiParams", {AnyValue(10), AnyValue(20), AnyValue(30)});
        if (response5.isSuccess()) {
            int result = response5.result.cast<int>();
            std::cout << "结果: " << result << " (期望: 60)" << std::endl;
            if (result != 60) all_tests_passed = false;
        } else {
            std::cout << "测试失败: " << response5.error_message << std::endl;
            all_tests_passed = false;
        }
        
        // 测试6: 无参数
        std::cout << "\n--- 测试无参数 ---" << std::endl;
        auto response6 = client.call("testNoParams");
        if (response6.isSuccess()) {
            std::string result = response6.result.cast<std::string>();
            std::cout << "结果: \"" << result << "\"" << std::endl;
            if (result != "No parameters test passed!") all_tests_passed = false;
        } else {
            std::cout << "测试失败: " << response6.error_message << std::endl;
            all_tests_passed = false;
        }
        
        // 测试7: 异步调用
        std::cout << "\n--- 测试异步调用 ---" << std::endl;
        auto future = client.callAsync("testInt", {AnyValue(100)});
        auto async_response = future.get();
        if (async_response.isSuccess()) {
            int result = async_response.result.cast<int>();
            std::cout << "异步结果: " << result << " (期望: 200)" << std::endl;
            if (result != 200) all_tests_passed = false;
        } else {
            std::cout << "异步测试失败: " << async_response.error_message << std::endl;
            all_tests_passed = false;
        }
        
        // 测试8: 错误处理
        std::cout << "\n--- 测试错误处理 ---" << std::endl;
        auto response8 = client.call("testError", {AnyValue(-5)});
        if (!response8.isSuccess()) {
            std::cout << "正确捕获错误: " << response8.error_message << std::endl;
        } else {
            std::cout << "错误处理测试失败: 应该返回错误但返回了成功" << std::endl;
            all_tests_passed = false;
        }
        
        // 测试9: 不存在的方法
        std::cout << "\n--- 测试不存在的方法 ---" << std::endl;
        auto response9 = client.call("nonExistentMethod");
        if (!response9.isSuccess()) {
            std::cout << "正确处理不存在的方法: " << response9.error_message << std::endl;
        } else {
            std::cout << "不存在方法测试失败: 应该返回错误但返回了成功" << std::endl;
            all_tests_passed = false;
        }
        
    } catch (const std::exception& e) {
        std::cout << "测试过程中发生异常: " << e.what() << std::endl;
        all_tests_passed = false;
    }
    
    // 显示统计信息
    std::cout << "\n--- 统计信息 ---" << std::endl;
    const auto& client_stats = client.getStatistics();
    std::cout << "客户端 - 总请求: " << client_stats.total_requests 
             << ", 成功: " << client_stats.successful_requests
             << ", 失败: " << client_stats.failed_requests << std::endl;
             
    const auto& server_stats = server.getStatistics();
    std::cout << "服务器 - 总请求: " << server_stats.total_requests 
             << ", 成功: " << server_stats.successful_requests
             << ", 失败: " << server_stats.failed_requests << std::endl;
    
    // 断开连接
    client.disconnect();
    server.stop();
    
    return all_tests_passed;
}

int main() {
    std::cout << "RPC框架全面测试程序" << std::endl;
    std::cout << "测试各种数据类型和功能特性" << std::endl;
    std::cout << "========================================" << std::endl;
    
    bool success = runComprehensiveTest();
    
    std::cout << "\n========================================" << std::endl;
    if (success) {
        std::cout << "✅ 所有测试通过！RPC框架工作正常。" << std::endl;
        return 0;
    } else {
        std::cout << "❌ 部分测试失败！请检查实现。" << std::endl;
        return 1;
    }
} 