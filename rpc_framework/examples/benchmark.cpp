#include "../include/rpc_client.h"
#include "../include/rpc_server.h"
#include "../include/transport.h"
#include "../include/serializer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <future>
#include <iomanip>

using namespace rpc;

// 基准测试服务类
class BenchmarkService {
public:
    // 简单的加法运算
    int add(int a, int b) {
        return a + b;
    }
    
    // 字符串处理
    std::string processString(const std::string& input) {
        return "Processed: " + input;
    }
    
    // 空操作（测试最小开销）
    void noop() {
        // 什么都不做
    }
    
    // 计算密集型操作
    int fibonacci(int n) {
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

// 基准测试配置
struct BenchmarkConfig {
    ProtocolType protocol = ProtocolType::TCP;
    SerializationType serialization = SerializationType::JSON;
    int num_threads = 1;
    int requests_per_thread = 1000;
    int server_port = 8082;
    std::string test_method = "add";
    bool verbose = false;
};

// 全局变量
std::atomic<bool> server_ready{false};
std::atomic<int> completed_requests{0};
std::atomic<int> failed_requests{0};

// 启动基准测试服务器
void runBenchmarkServer(const BenchmarkConfig& config) {
    std::cout << "启动基准测试服务器..." << std::endl;
    
    RpcServer server(config.protocol, config.serialization);
    
    auto service = std::make_shared<BenchmarkService>();
    ServiceRegistrar<BenchmarkService> registrar(server, service);
    
    // 注册所有测试方法
    registrar.registerMethod("add", &BenchmarkService::add);
    registrar.registerMethod("processString", &BenchmarkService::processString);
    registrar.registerMethod("noop", &BenchmarkService::noop);
    registrar.registerMethod("fibonacci", &BenchmarkService::fibonacci);
    
    // 设置较大的线程池
    server.setThreadPoolSize(std::max(4, config.num_threads));
    server.setRequestQueueSize(10000);
    
    ServiceEndpoint endpoint("127.0.0.1", config.server_port);
    if (server.start(endpoint)) {
        std::cout << "基准测试服务器启动成功，监听端口 " << config.server_port << std::endl;
        server_ready = true;
        
        // 保持服务器运行
        while (server_ready) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 显示最终统计
        const auto& stats = server.getStatistics();
        std::cout << "\n服务器最终统计:" << std::endl;
        std::cout << "  总请求: " << stats.total_requests << std::endl;
        std::cout << "  成功请求: " << stats.successful_requests << std::endl;
        std::cout << "  失败请求: " << stats.failed_requests << std::endl;
        std::cout << "  平均响应时间: " << stats.avg_response_time_ms << "ms" << std::endl;
    } else {
        std::cout << "基准测试服务器启动失败!" << std::endl;
    }
}

// 单个线程的基准测试
void benchmarkThread(const BenchmarkConfig& config, int thread_id, 
                    std::vector<double>& latencies) {
    
    RpcClient client(config.protocol, config.serialization);
    ServiceEndpoint endpoint("127.0.0.1", config.server_port);
    
    if (!client.connect(endpoint)) {
        std::cerr << "线程 " << thread_id << " 连接失败" << std::endl;
        return;
    }
    
    latencies.reserve(config.requests_per_thread);
    
    for (int i = 0; i < config.requests_per_thread; ++i) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        try {
            RpcResponse response;
            
            if (config.test_method == "add") {
                std::vector<AnyValue> params = {AnyValue(i), AnyValue(i + 1)};
                response = client.call("add", params);
            } else if (config.test_method == "processString") {
                std::vector<AnyValue> params = {AnyValue(std::string("test_" + std::to_string(i)))};
                response = client.call("processString", params);
            } else if (config.test_method == "noop") {
                response = client.call("noop");
            } else if (config.test_method == "fibonacci") {
                std::vector<AnyValue> params = {AnyValue(10)};
                response = client.call("fibonacci", params);
            }
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                end_time - start_time).count();
            
            if (response.isSuccess()) {
                latencies.push_back(duration / 1000.0); // 转换为毫秒
                completed_requests++;
            } else {
                failed_requests++;
                if (config.verbose) {
                    std::cout << "请求失败: " << response.error_message << std::endl;
                }
            }
            
        } catch (const std::exception& e) {
            failed_requests++;
            if (config.verbose) {
                std::cout << "异常: " << e.what() << std::endl;
            }
        }
        
        // 进度显示
        if (config.verbose && (i + 1) % 100 == 0) {
            std::cout << "线程 " << thread_id << " 完成 " << (i + 1) << " 请求" << std::endl;
        }
    }
    
    client.disconnect();
}

// 计算统计信息
void calculateStatistics(const std::vector<std::vector<double>>& all_latencies,
                        const BenchmarkConfig& config,
                        std::chrono::milliseconds total_time) {
    
    // 合并所有延迟数据
    std::vector<double> all_times;
    for (const auto& thread_latencies : all_latencies) {
        all_times.insert(all_times.end(), thread_latencies.begin(), thread_latencies.end());
    }
    
    if (all_times.empty()) {
        std::cout << "没有成功的请求!" << std::endl;
        return;
    }
    
    // 排序以计算百分位数
    std::sort(all_times.begin(), all_times.end());
    
    double total_latency = 0;
    for (double latency : all_times) {
        total_latency += latency;
    }
    
    double avg_latency = total_latency / all_times.size();
    double min_latency = all_times.front();
    double max_latency = all_times.back();
    double p50 = all_times[all_times.size() * 0.5];
    double p95 = all_times[all_times.size() * 0.95];
    double p99 = all_times[all_times.size() * 0.99];
    
    int total_requests = config.num_threads * config.requests_per_thread;
    double rps = (double)completed_requests / (total_time.count() / 1000.0);
    
    // 输出结果
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "基准测试结果" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::cout << "测试配置:" << std::endl;
    std::cout << "  协议: " << protocolTypeToString(config.protocol) << std::endl;
    std::cout << "  序列化: " << serializationTypeToString(config.serialization) << std::endl;
    std::cout << "  线程数: " << config.num_threads << std::endl;
    std::cout << "  每线程请求数: " << config.requests_per_thread << std::endl;
    std::cout << "  测试方法: " << config.test_method << std::endl;
    std::cout << std::endl;
    
    std::cout << "性能指标:" << std::endl;
    std::cout << "  总请求数: " << total_requests << std::endl;
    std::cout << "  成功请求: " << completed_requests << std::endl;
    std::cout << "  失败请求: " << failed_requests << std::endl;
    std::cout << "  成功率: " << std::fixed << std::setprecision(2) 
              << (100.0 * completed_requests / total_requests) << "%" << std::endl;
    std::cout << "  总耗时: " << total_time.count() << "ms" << std::endl;
    std::cout << "  QPS: " << std::fixed << std::setprecision(0) << rps << std::endl;
    std::cout << std::endl;
    
    std::cout << "延迟统计 (毫秒):" << std::endl;
    std::cout << "  最小值: " << std::fixed << std::setprecision(3) << min_latency << std::endl;
    std::cout << "  最大值: " << std::fixed << std::setprecision(3) << max_latency << std::endl;
    std::cout << "  平均值: " << std::fixed << std::setprecision(3) << avg_latency << std::endl;
    std::cout << "  P50: " << std::fixed << std::setprecision(3) << p50 << std::endl;
    std::cout << "  P95: " << std::fixed << std::setprecision(3) << p95 << std::endl;
    std::cout << "  P99: " << std::fixed << std::setprecision(3) << p99 << std::endl;
    
    std::cout << std::string(60, '=') << std::endl;
}

// 运行基准测试
void runBenchmark(const BenchmarkConfig& config) {
    std::cout << "准备运行基准测试..." << std::endl;
    
    // 启动服务器线程
    std::thread server_thread(runBenchmarkServer, std::ref(config));
    
    // 等待服务器启动
    while (!server_ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 额外等待
    
    std::cout << "开始基准测试..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 启动客户端线程
    std::vector<std::thread> client_threads;
    std::vector<std::vector<double>> all_latencies(config.num_threads);
    
    for (int i = 0; i < config.num_threads; ++i) {
        client_threads.emplace_back(benchmarkThread, std::ref(config), i, 
                                   std::ref(all_latencies[i]));
    }
    
    // 等待所有客户端线程完成
    for (auto& thread : client_threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    
    // 停止服务器
    server_ready = false;
    server_thread.join();
    
    // 计算并显示统计信息
    calculateStatistics(all_latencies, config, total_time);
}

// 解析命令行参数
BenchmarkConfig parseArgs(int argc, char* argv[]) {
    BenchmarkConfig config;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--protocol" && i + 1 < argc) {
            std::string protocol = argv[++i];
            if (protocol == "tcp") config.protocol = ProtocolType::TCP;
            else if (protocol == "http") config.protocol = ProtocolType::HTTP;
            else if (protocol == "udp") config.protocol = ProtocolType::UDP;
        } else if (arg == "--serialization" && i + 1 < argc) {
            std::string serialization = argv[++i];
            if (serialization == "json") config.serialization = SerializationType::JSON;
            else if (serialization == "binary") config.serialization = SerializationType::BINARY;
            else if (serialization == "msgpack") config.serialization = SerializationType::MESSAGEPACK;
        } else if (arg == "--threads" && i + 1 < argc) {
            config.num_threads = std::stoi(argv[++i]);
        } else if (arg == "--requests" && i + 1 < argc) {
            config.requests_per_thread = std::stoi(argv[++i]);
        } else if (arg == "--method" && i + 1 < argc) {
            config.test_method = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            config.server_port = std::stoi(argv[++i]);
        } else if (arg == "--verbose") {
            config.verbose = true;
        } else if (arg == "--help") {
            std::cout << "RPC框架基准测试工具" << std::endl;
            std::cout << "用法: " << argv[0] << " [选项]" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  --protocol <tcp|http|udp>     协议类型 (默认: tcp)" << std::endl;
            std::cout << "  --serialization <json|binary|msgpack> 序列化格式 (默认: json)" << std::endl;
            std::cout << "  --threads <数量>              并发线程数 (默认: 1)" << std::endl;
            std::cout << "  --requests <数量>             每线程请求数 (默认: 1000)" << std::endl;
            std::cout << "  --method <方法名>             测试方法 (默认: add)" << std::endl;
            std::cout << "  --port <端口>                 服务器端口 (默认: 8082)" << std::endl;
            std::cout << "  --verbose                     详细输出" << std::endl;
            std::cout << "  --help                        显示帮助" << std::endl;
            exit(0);
        }
    }
    
    return config;
}

int main(int argc, char* argv[]) {
    std::cout << "RPC框架基准测试工具" << std::endl;
    std::cout << "版本: 1.0.0" << std::endl;
    std::cout << std::string(40, '-') << std::endl;
    
    BenchmarkConfig config = parseArgs(argc, argv);
    
    try {
        runBenchmark(config);
    } catch (const std::exception& e) {
        std::cerr << "基准测试异常: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}

// 编译说明:
// g++ -std=c++14 -pthread -O2 -I../include benchmark.cpp ../src/*.cpp -o benchmark
//
// 使用示例:
// ./benchmark --protocol=tcp --serialization=json --threads=4 --requests=1000
// ./benchmark --protocol=http --serialization=json --threads=2 --requests=500 --verbose
// ./benchmark --method=fibonacci --threads=1 --requests=100 