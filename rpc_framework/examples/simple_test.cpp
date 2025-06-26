#include "../include/rpc_client.h"
#include "../include/rpc_server.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace rpc;

// 简单的测试服务
class TestService {
public:
    std::string hello() {
        std::cout << "服务器: 执行hello方法" << std::endl;
        return "Hello from RPC Server!";
    }
    
    int add(int a, int b) {
        std::cout << "服务器: 执行加法 " << a << " + " << b << std::endl;
        return a + b;
    }
};

int main() {
    std::cout << "=== RPC框架简单测试 ===" << std::endl;
    
    try {
        // 创建服务器
        RpcServer server(ProtocolType::TCP, SerializationType::JSON);
        
        // 创建服务实例
        auto service = std::make_shared<TestService>();
        
        // 注册方法
        server.registerMethod("hello", [service](const std::vector<AnyValue>& params) -> AnyValue {
            if (!params.empty()) {
                throw std::invalid_argument("hello方法不需要参数");
            }
            std::string result = service->hello();
            return AnyValue(result);
        });
        
        server.registerMethod("add", [service](const std::vector<AnyValue>& params) -> AnyValue {
            if (params.size() != 2) {
                throw std::invalid_argument("add方法需要2个参数");
            }
            
            #if HAS_STD_ANY
            int a = std::any_cast<int>(params[0]);
            int b = std::any_cast<int>(params[1]);
            #else
            int a = params[0].cast<int>();
            int b = params[1].cast<int>();
            #endif
            
            int result = service->add(a, b);
            return AnyValue(result);
        });
        
        // 启动服务器
        ServiceEndpoint endpoint("127.0.0.1", 8080);
        if (!server.start(endpoint)) {
            std::cout << "服务器启动失败!" << std::endl;
            return 1;
        }
        
        std::cout << "服务器启动成功，监听 " << endpoint.toString() << std::endl;
        
        // 等待服务器启动
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 创建客户端
        RpcClient client(ProtocolType::TCP, SerializationType::JSON);
        
        // 连接到服务器
        if (!client.connect(endpoint)) {
            std::cout << "连接服务器失败!" << std::endl;
            return 1;
        }
        
        std::cout << "客户端连接成功!" << std::endl;
        
        // 测试hello方法
        std::cout << "\n--- 测试hello方法 ---" << std::endl;
        auto response1 = client.call("hello");
        if (response1.isSuccess()) {
            #if HAS_STD_ANY
            auto result = std::any_cast<std::string>(response1.result);
            #else
            auto result = response1.result.cast<std::string>();
            #endif
            std::cout << "结果: " << result << std::endl;
        } else {
            std::cout << "调用失败: " << response1.error_message << std::endl;
        }
        
        // 测试add方法
        std::cout << "\n--- 测试add方法 ---" << std::endl;
        std::vector<AnyValue> params = {AnyValue(10), AnyValue(20)};
        auto response2 = client.call("add", params);
        if (response2.isSuccess()) {
            #if HAS_STD_ANY
            auto result = std::any_cast<int>(response2.result);
            #else
            auto result = response2.result.cast<int>();
            #endif
            std::cout << "结果: 10 + 20 = " << result << std::endl;
        } else {
            std::cout << "调用失败: " << response2.error_message << std::endl;
        }
        
        std::cout << "\n测试完成!" << std::endl;
        
        // 清理
        client.disconnect();
        server.stop();
        
    } catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 