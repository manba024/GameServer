#pragma once

#include "rpc_types.h"
#include "transport.h"
#include "serializer.h"
#include <memory>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>

namespace rpc {

// 方法调用上下文
struct CallContext {
    std::string client_id;
    std::string request_id;
    std::map<std::string, std::string> headers;
    std::chrono::steady_clock::time_point start_time;
};

// 高级方法处理器（带上下文）
using AdvancedMethodHandler = std::function<AnyValue(const std::vector<AnyValue>&, const CallContext&)>;

// 中间件类型
using Middleware = std::function<bool(const RpcRequest&, RpcResponse&, const CallContext&)>;

// RPC服务器
class RpcServer {
public:
    RpcServer(ProtocolType protocol = ProtocolType::TCP,
              SerializationType serialization = SerializationType::JSON);
    ~RpcServer();
    
    // 启动服务器
    bool start(const ServiceEndpoint& endpoint);
    
    // 停止服务器
    void stop();
    
    // 检查服务器状态
    bool isRunning() const;
    
    // 注册方法
    void registerMethod(const std::string& method_name, MethodHandler handler);
    void registerMethod(const std::string& method_name, AdvancedMethodHandler handler);
    
    // 取消注册方法
    void unregisterMethod(const std::string& method_name);
    
    // 注册中间件
    void addMiddleware(Middleware middleware);
    
    // 设置线程池大小
    void setThreadPoolSize(size_t size);
    
    // 设置请求队列大小
    void setRequestQueueSize(size_t size);
    
    // 获取统计信息
    struct Statistics {
        std::atomic<uint64_t> total_requests{0};
        std::atomic<uint64_t> successful_requests{0};
        std::atomic<uint64_t> failed_requests{0};
        std::atomic<uint64_t> active_connections{0};
        std::atomic<uint64_t> bytes_received{0};
        std::atomic<uint64_t> bytes_sent{0};
        std::atomic<uint64_t> avg_response_time_ms{0};
    };
    
    const Statistics& getStatistics() const { return stats_; }
    void resetStatistics();
    
    // 设置错误处理器
    void setErrorHandler(std::function<void(const std::string&, ErrorCode)> handler);

private:
    std::unique_ptr<ServerTransport> transport_;
    std::unique_ptr<Serializer> serializer_;
    ServiceEndpoint current_endpoint_;
    
    // 方法注册表
    std::unordered_map<std::string, MethodHandler> methods_;
    std::unordered_map<std::string, AdvancedMethodHandler> advanced_methods_;
    std::mutex methods_mutex_;
    
    // 中间件
    std::vector<Middleware> middlewares_;
    std::mutex middlewares_mutex_;
    
    // 线程池
    std::vector<std::thread> worker_threads_;
    std::queue<std::function<void()>> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_condition_;
    std::atomic<bool> running_;
    size_t thread_pool_size_;
    size_t max_queue_size_;
    
    // 统计信息
    Statistics stats_;
    std::function<void(const std::string&, ErrorCode)> error_handler_;
    
    // 内部方法
    std::string handleRequest(const std::string& request_data);
    RpcResponse processRequest(const RpcRequest& request, const CallContext& context);
    void workerThread();
    bool runMiddlewares(const RpcRequest& request, RpcResponse& response, const CallContext& context);
    void updateStatistics(const std::chrono::steady_clock::time_point& start_time, bool success);
    
    // 工具方法
    template<typename T>
    AnyValue makeAnyValue(T&& value) {
        return AnyValue(std::forward<T>(value));
    }
};

// 简化的服务注册器（用于手动注册类方法）
template<typename ServiceClass>
class ServiceRegistrar {
public:
    ServiceRegistrar(RpcServer& server, std::shared_ptr<ServiceClass> service_instance)
        : server_(server), service_(service_instance) {}
    
    // 注册无参数方法
    template<typename ReturnType>
    void registerMethod(const std::string& method_name, 
                       ReturnType (ServiceClass::*method)()) {
        server_.registerMethod(method_name, [this, method](const std::vector<AnyValue>& params) -> AnyValue {
            if (!params.empty()) {
                throw std::invalid_argument("Method expects no parameters");
            }
            return callNoArgMethod(method);
        });
    }
    
    // 注册单参数方法
    template<typename ReturnType, typename Arg1>
    void registerMethod(const std::string& method_name,
                       ReturnType (ServiceClass::*method)(Arg1)) {
        server_.registerMethod(method_name, [this, method](const std::vector<AnyValue>& params) -> AnyValue {
            if (params.size() != 1) {
                throw std::invalid_argument("Method expects 1 parameter");
            }
            return callOneArgMethod(method, extractParam<Arg1>(params[0]));
        });
    }
    
    // 注册双参数方法
    template<typename ReturnType, typename Arg1, typename Arg2>
    void registerMethod(const std::string& method_name,
                       ReturnType (ServiceClass::*method)(Arg1, Arg2)) {
        server_.registerMethod(method_name, [this, method](const std::vector<AnyValue>& params) -> AnyValue {
            if (params.size() != 2) {
                throw std::invalid_argument("Method expects 2 parameters");
            }
            return callTwoArgMethod(method, extractParam<Arg1>(params[0]), extractParam<Arg2>(params[1]));
        });
    }
    
    // 注册三参数方法
    template<typename ReturnType, typename Arg1, typename Arg2, typename Arg3>
    void registerMethod(const std::string& method_name,
                       ReturnType (ServiceClass::*method)(Arg1, Arg2, Arg3)) {
        server_.registerMethod(method_name, [this, method](const std::vector<AnyValue>& params) -> AnyValue {
            if (params.size() != 3) {
                throw std::invalid_argument("Method expects 3 parameters");
            }
            return callThreeArgMethod(method, extractParam<Arg1>(params[0]), 
                                    extractParam<Arg2>(params[1]), extractParam<Arg3>(params[2]));
        });
    }

private:
    RpcServer& server_;
    std::shared_ptr<ServiceClass> service_;
    
    template<typename ReturnType>
    AnyValue callNoArgMethod(ReturnType (ServiceClass::*method)()) {
        auto result = (service_.get()->*method)();
        return AnyValue(result);
    }
    
    // void返回类型特化
    AnyValue callNoArgMethod(void (ServiceClass::*method)()) {
        (service_.get()->*method)();
        return AnyValue();
    }
    
    template<typename ReturnType, typename Arg1>
    AnyValue callOneArgMethod(ReturnType (ServiceClass::*method)(Arg1), typename std::decay<Arg1>::type arg1) {
        auto result = (service_.get()->*method)(arg1);
        return AnyValue(result);
    }
    
    template<typename Arg1>
    AnyValue callOneArgMethod(void (ServiceClass::*method)(Arg1), typename std::decay<Arg1>::type arg1) {
        (service_.get()->*method)(arg1);
        return AnyValue();
    }
    
    template<typename ReturnType, typename Arg1, typename Arg2>
    AnyValue callTwoArgMethod(ReturnType (ServiceClass::*method)(Arg1, Arg2), 
                             typename std::decay<Arg1>::type arg1, typename std::decay<Arg2>::type arg2) {
        auto result = (service_.get()->*method)(arg1, arg2);
        return AnyValue(result);
    }
    
    template<typename Arg1, typename Arg2>
    AnyValue callTwoArgMethod(void (ServiceClass::*method)(Arg1, Arg2), 
                             typename std::decay<Arg1>::type arg1, typename std::decay<Arg2>::type arg2) {
        (service_.get()->*method)(arg1, arg2);
        return AnyValue();
    }
    
    template<typename ReturnType, typename Arg1, typename Arg2, typename Arg3>
    AnyValue callThreeArgMethod(ReturnType (ServiceClass::*method)(Arg1, Arg2, Arg3), 
                               typename std::decay<Arg1>::type arg1, typename std::decay<Arg2>::type arg2, 
                               typename std::decay<Arg3>::type arg3) {
        auto result = (service_.get()->*method)(arg1, arg2, arg3);
        return AnyValue(result);
    }
    
    template<typename Arg1, typename Arg2, typename Arg3>
    AnyValue callThreeArgMethod(void (ServiceClass::*method)(Arg1, Arg2, Arg3), 
                               typename std::decay<Arg1>::type arg1, typename std::decay<Arg2>::type arg2, 
                               typename std::decay<Arg3>::type arg3) {
        (service_.get()->*method)(arg1, arg2, arg3);
        return AnyValue();
    }
    
    template<typename T>
    typename std::decay<T>::type extractParam(const AnyValue& value) {
        #if HAS_STD_ANY
        return std::any_cast<typename std::decay<T>::type>(value);
        #else
        return value.cast<T>();
        #endif
    }
};

// 服务发现接口
class ServiceDiscovery {
public:
    virtual ~ServiceDiscovery() = default;
    
    // 注册服务
    virtual bool registerService(const std::string& service_name, 
                                const ServiceEndpoint& endpoint) = 0;
    
    // 取消注册服务
    virtual bool unregisterService(const std::string& service_name,
                                  const ServiceEndpoint& endpoint) = 0;
    
    // 发现服务
    virtual std::vector<ServiceEndpoint> discoverService(const std::string& service_name) = 0;
    
    // 设置服务发现回调
    virtual void setDiscoveryCallback(const std::string& service_name,
                                     ServiceDiscoveryCallback callback) = 0;
};

// 简单的内存服务发现实现
class MemoryServiceDiscovery : public ServiceDiscovery {
public:
    bool registerService(const std::string& service_name, 
                        const ServiceEndpoint& endpoint) override;
    
    bool unregisterService(const std::string& service_name,
                          const ServiceEndpoint& endpoint) override;
    
    std::vector<ServiceEndpoint> discoverService(const std::string& service_name) override;
    
    void setDiscoveryCallback(const std::string& service_name,
                             ServiceDiscoveryCallback callback) override;

private:
    std::unordered_map<std::string, std::vector<ServiceEndpoint>> services_;
    std::unordered_map<std::string, ServiceDiscoveryCallback> callbacks_;
    mutable std::mutex mutex_;
    
    void notifyCallbacks(const std::string& service_name);
};

// RPC服务器集群管理器
class RpcServerCluster {
public:
    RpcServerCluster(std::shared_ptr<ServiceDiscovery> discovery);
    ~RpcServerCluster();
    
    // 添加服务器实例
    void addServer(const std::string& service_name, 
                   std::unique_ptr<RpcServer> server,
                   const ServiceEndpoint& endpoint);
    
    // 移除服务器实例
    void removeServer(const std::string& service_name);
    
    // 启动所有服务器
    bool startAll();
    
    // 停止所有服务器
    void stopAll();
    
    // 获取集群状态
    struct ClusterStatus {
        size_t total_servers;
        size_t running_servers;
        size_t stopped_servers;
    };
    
    ClusterStatus getStatus() const;

private:
    std::shared_ptr<ServiceDiscovery> discovery_;
    
    struct ServerInfo {
        std::string service_name;
        std::unique_ptr<RpcServer> server;
        ServiceEndpoint endpoint;
        bool running;
    };
    
    std::vector<ServerInfo> servers_;
    mutable std::mutex servers_mutex_;
};

} // namespace rpc 