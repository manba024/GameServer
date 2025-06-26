#include "../include/rpc_client.h"
#include "../include/rpc_server.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <vector>
#include <future>
#include <atomic>

using namespace rpc;

// 简单的计算服务
class CalculatorService {
public:
    int add(int a, int b) {
        // 模拟一些计算时间
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        return a + b;
    }
    
    int multiply(int a, int b) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        return a * b;
    }
};

void clientWorker(int worker_id, int num_requests, 
                 std::atomic<int>& total_success, 
                 std::atomic<int>& total_failed,
                 std::chrono::steady_clock::time_point start_time) {
    
    RpcClient client(ProtocolType::TCP, SerializationType::JSON);
    ServiceEndpoint endpoint("127.0.0.1", 8086);
    
    if (!client.connect(endpoint)) {
        std::cout << "Worker " << worker_id << " 连接失败" << std::endl;
        total_failed += num_requests;
        return;
    }
    
    int success_count = 0;
    int failed_count = 0;
    
    for (int i = 0; i < num_requests; ++i) {
        try {
            // 交替调用不同方法
            if (i % 2 == 0) {
                auto response = client.call("add", {AnyValue(i), AnyValue(worker_id)});
                if (response.isSuccess()) {
                    int result = response.result.cast<int>();
                    if (result == i + worker_id) {
                        success_count++;
                    } else {
                        failed_count++;
                    }
                } else {
                    failed_count++;
                }
            } else {
                auto response = client.call("multiply", {AnyValue(i), AnyValue(2)});
                if (response.isSuccess()) {
                    int result = response.result.cast<int>();
                    if (result == i * 2) {
                        success_count++;
                    } else {
                        failed_count++;
                    }
                } else {
                    failed_count++;
                }
            }
        } catch (const std::exception& e) {
            failed_count++;
        }
    }
    
    client.disconnect();
    
    total_success += success_count;
    total_failed += failed_count;
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Worker " << worker_id << " 完成: " 
              << success_count << " 成功, " 
              << failed_count << " 失败, 耗时: " 
              << duration.count() << "ms" << std::endl;
}

bool runConcurrentTest() {
    std::cout << "=== 启动并发测试 ===" << std::endl;
    
    // 创建服务器
    RpcServer server(ProtocolType::TCP, SerializationType::JSON);
    auto service = std::make_shared<CalculatorService>();
    
    ServiceRegistrar<CalculatorService> registrar(server, service);
    registrar.registerMethod("add", &CalculatorService::add);
    registrar.registerMethod("multiply", &CalculatorService::multiply);
    
    // 启动服务器
    ServiceEndpoint endpoint("127.0.0.1", 8086);
    if (!server.start(endpoint)) {
        std::cout << "服务器启动失败!" << std::endl;
        return false;
    }
    
    std::cout << "服务器启动成功，监听 " << endpoint.toString() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 并发测试参数
    const int num_workers = 10;
    const int requests_per_worker = 100;
    
    std::atomic<int> total_success{0};
    std::atomic<int> total_failed{0};
    
    auto start_time = std::chrono::steady_clock::now();
    
    // 启动多个工作线程
    std::vector<std::thread> workers;
    for (int i = 0; i < num_workers; ++i) {
        workers.emplace_back(clientWorker, i, requests_per_worker, 
                           std::ref(total_success), std::ref(total_failed), start_time);
    }
    
    // 等待所有工作线程完成
    for (auto& worker : workers) {
        worker.join();
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 显示结果
    int total_requests = num_workers * requests_per_worker;
    double success_rate = (double)total_success / total_requests * 100.0;
    double qps = (double)total_requests / total_duration.count() * 1000.0;
    
    std::cout << "\n=== 并发测试结果 ===" << std::endl;
    std::cout << "工作线程数: " << num_workers << std::endl;
    std::cout << "每线程请求数: " << requests_per_worker << std::endl;
    std::cout << "总请求数: " << total_requests << std::endl;
    std::cout << "成功请求: " << total_success << std::endl;
    std::cout << "失败请求: " << total_failed << std::endl;
    std::cout << "成功率: " << std::fixed << std::setprecision(2) << success_rate << "%" << std::endl;
    std::cout << "总耗时: " << total_duration.count() << "ms" << std::endl;
    std::cout << "QPS: " << std::fixed << std::setprecision(0) << qps << std::endl;
    
    // 显示服务器统计
    const auto& server_stats = server.getStatistics();
    std::cout << "\n=== 服务器统计 ===" << std::endl;
    std::cout << "总请求: " << server_stats.total_requests << std::endl;
    std::cout << "成功请求: " << server_stats.successful_requests << std::endl;
    std::cout << "失败请求: " << server_stats.failed_requests << std::endl;
    std::cout << "活跃连接: " << server_stats.active_connections << std::endl;
    
    server.stop();
    
    return success_rate >= 95.0; // 95%以上成功率认为测试通过
}

int main() {
    std::cout << "RPC框架并发性能测试" << std::endl;
    std::cout << "测试多线程客户端同时访问服务器" << std::endl;
    std::cout << "========================================" << std::endl;
    
    bool success = runConcurrentTest();
    
    std::cout << "\n========================================" << std::endl;
    if (success) {
        std::cout << "✅ 并发测试通过！RPC框架具有良好的并发性能。" << std::endl;
        return 0;
    } else {
        std::cout << "❌ 并发测试失败！成功率过低。" << std::endl;
        return 1;
    }
} 