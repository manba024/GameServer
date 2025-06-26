#pragma once

#include "rpc_types.h"
#include "transport.h"
#include "serializer.h"
#include <memory>
#include <future>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace rpc {

// RPC客户端
class RpcClient {
public:
    RpcClient(ProtocolType protocol = ProtocolType::TCP, 
              SerializationType serialization = SerializationType::JSON);
    ~RpcClient();
    
    // 连接到服务器
    bool connect(const ServiceEndpoint& endpoint);
    
    // 断开连接
    void disconnect();
    
    // 检查连接状态
    bool isConnected() const;
    
    // 同步调用
    RpcResponse call(const std::string& method, const std::vector<AnyValue>& params = {});
    
    // 异步调用
    std::future<RpcResponse> callAsync(const std::string& method, 
                                      const std::vector<AnyValue>& params = {});
    
    // 异步调用（带回调）
    void callAsync(const std::string& method, 
                   const std::vector<AnyValue>& params,
                   AsyncCallback callback);
    
    // 单向调用（不等待响应）
    bool callOneWay(const std::string& method, const std::vector<AnyValue>& params = {});
    
    // 设置超时时间
    void setTimeout(std::chrono::milliseconds timeout);
    
    // 设置连接状态回调
    void setConnectionCallback(ConnectionCallback callback);
    
    // 获取统计信息
    struct Statistics {
        std::atomic<uint64_t> total_requests{0};
        std::atomic<uint64_t> successful_requests{0};
        std::atomic<uint64_t> failed_requests{0};
        std::atomic<uint64_t> timeout_requests{0};
        std::atomic<uint64_t> bytes_sent{0};
        std::atomic<uint64_t> bytes_received{0};
    };
    
    const Statistics& getStatistics() const { return stats_; }
    void resetStatistics();

private:
    std::unique_ptr<Transport> transport_;
    std::unique_ptr<Serializer> serializer_;
    ServiceEndpoint current_endpoint_;
    std::chrono::milliseconds timeout_;
    
    // 请求管理
    std::atomic<uint32_t> next_request_id_;
    std::unordered_map<std::string, std::promise<RpcResponse>> pending_requests_;
    std::unordered_map<std::string, AsyncCallback> async_callbacks_;
    std::mutex requests_mutex_;
    
    // 响应处理线程
    std::thread response_thread_;
    std::atomic<bool> running_;
    
    // 统计信息
    Statistics stats_;
    
    // 内部方法
    std::string generateRequestId();
    void responseHandler();
    RpcResponse processResponse(const std::string& response_data);
    void handleAsyncResponse(const RpcResponse& response);
    
    // 工具方法
    template<typename T>
    T extractValue(const AnyValue& value) {
        #if HAS_STD_ANY
        return std::any_cast<T>(value);
        #else
        return value.cast<T>();
        #endif
    }
};

// RPC客户端池（用于连接复用）
class RpcClientPool {
public:
    RpcClientPool(const ServiceEndpoint& endpoint, 
                  size_t pool_size = 10,
                  ProtocolType protocol = ProtocolType::TCP,
                  SerializationType serialization = SerializationType::JSON);
    ~RpcClientPool();
    
    // 获取客户端（自动管理连接）
    std::shared_ptr<RpcClient> getClient();
    
    // 释放客户端
    void releaseClient(std::shared_ptr<RpcClient> client);
    
    // 设置池大小
    void setPoolSize(size_t size);
    
    // 获取池状态
    struct PoolStatus {
        size_t total_clients;
        size_t available_clients;
        size_t busy_clients;
    };
    
    PoolStatus getStatus() const;

private:
    ServiceEndpoint endpoint_;
    ProtocolType protocol_;
    SerializationType serialization_;
    size_t max_pool_size_;
    
    std::vector<std::shared_ptr<RpcClient>> available_clients_;
    std::vector<std::shared_ptr<RpcClient>> busy_clients_;
    mutable std::mutex pool_mutex_;
    std::condition_variable pool_condition_;
    
    std::shared_ptr<RpcClient> createClient();
};

// 负载均衡RPC客户端
class LoadBalancedRpcClient {
public:
    enum class LoadBalanceStrategy {
        ROUND_ROBIN,    // 轮询
        RANDOM,         // 随机
        LEAST_CONNECTIONS, // 最少连接
        WEIGHTED_ROUND_ROBIN // 加权轮询
    };
    
    LoadBalancedRpcClient(LoadBalanceStrategy strategy = LoadBalanceStrategy::ROUND_ROBIN);
    ~LoadBalancedRpcClient();
    
    // 添加服务端点
    void addEndpoint(const ServiceEndpoint& endpoint, int weight = 1);
    
    // 移除服务端点
    void removeEndpoint(const ServiceEndpoint& endpoint);
    
    // 设置负载均衡策略
    void setLoadBalanceStrategy(LoadBalanceStrategy strategy);
    
    // RPC调用（自动负载均衡）
    RpcResponse call(const std::string& method, const std::vector<AnyValue>& params = {});
    std::future<RpcResponse> callAsync(const std::string& method, 
                                      const std::vector<AnyValue>& params = {});
    
    // 设置超时时间
    void setTimeout(std::chrono::milliseconds timeout);

private:
    LoadBalanceStrategy strategy_;
    std::chrono::milliseconds timeout_;
    
    struct EndpointInfo {
        ServiceEndpoint endpoint;
        int weight;
        std::shared_ptr<RpcClientPool> client_pool;
        std::atomic<int> active_connections{0};
    };
    
    std::vector<EndpointInfo> endpoints_;
    std::atomic<size_t> round_robin_index_{0};
    mutable std::mutex endpoints_mutex_;
    
    EndpointInfo* selectEndpoint();
    EndpointInfo* selectRoundRobin();
    EndpointInfo* selectRandom();
    EndpointInfo* selectLeastConnections();
    EndpointInfo* selectWeightedRoundRobin();
};

} // namespace rpc 