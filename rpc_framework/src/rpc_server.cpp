#include "../include/rpc_server.h"
#include "../include/transport.h"
#include "../include/serializer.h"
#include <iostream>

namespace rpc {

RpcServer::RpcServer(ProtocolType protocol, SerializationType serialization)
    : current_endpoint_("", 0), running_(false), thread_pool_size_(4), max_queue_size_(1000) {
    
    // 创建传输层
    switch (protocol) {
        case ProtocolType::TCP:
            transport_ = std::make_unique<TcpServerTransport>();
            break;
        default:
            throw std::runtime_error("Unsupported protocol type");
    }
    
    // 创建序列化器
    switch (serialization) {
        case SerializationType::JSON:
            serializer_ = std::make_unique<JsonSerializer>();
            break;
        default:
            throw std::runtime_error("Unsupported serialization type");
    }
}

RpcServer::~RpcServer() {
    stop();
}

bool RpcServer::start(const ServiceEndpoint& endpoint) {
    if (running_) {
        return false;
    }
    
    current_endpoint_ = endpoint;
    
    // 启动传输层
    if (!transport_->start(endpoint)) {
        return false;
    }
    
    // 设置消息处理器
    transport_->setMessageHandler([this](const std::string& request) {
        return handleRequest(request);
    });
    
    running_ = true;
    
    // 启动工作线程
    for (size_t i = 0; i < thread_pool_size_; ++i) {
        worker_threads_.emplace_back(&RpcServer::workerThread, this);
    }
    
    std::cout << "RPC服务器启动在 " << endpoint.toString() << std::endl;
    return true;
}

void RpcServer::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // 通知所有工作线程退出
    queue_condition_.notify_all();
    
    // 等待工作线程结束
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads_.clear();
    
    // 停止传输层
    transport_->stop();
    
    std::cout << "RPC服务器已停止" << std::endl;
}

bool RpcServer::isRunning() const {
    return running_;
}

void RpcServer::registerMethod(const std::string& method_name, MethodHandler handler) {
    std::lock_guard<std::mutex> lock(methods_mutex_);
    methods_[method_name] = handler;
}

void RpcServer::registerMethod(const std::string& method_name, AdvancedMethodHandler handler) {
    std::lock_guard<std::mutex> lock(methods_mutex_);
    advanced_methods_[method_name] = handler;
}

void RpcServer::unregisterMethod(const std::string& method_name) {
    std::lock_guard<std::mutex> lock(methods_mutex_);
    methods_.erase(method_name);
    advanced_methods_.erase(method_name);
}

void RpcServer::addMiddleware(Middleware middleware) {
    std::lock_guard<std::mutex> lock(middlewares_mutex_);
    middlewares_.push_back(middleware);
}

void RpcServer::setThreadPoolSize(size_t size) {
    thread_pool_size_ = size;
}

void RpcServer::setRequestQueueSize(size_t size) {
    max_queue_size_ = size;
}

void RpcServer::setErrorHandler(std::function<void(const std::string&, ErrorCode)> handler) {
    error_handler_ = handler;
}

void RpcServer::resetStatistics() {
    stats_.total_requests = 0;
    stats_.successful_requests = 0; 
    stats_.failed_requests = 0;
    stats_.active_connections = 0;
    stats_.bytes_received = 0;
    stats_.bytes_sent = 0;
    stats_.avg_response_time_ms = 0;
}

void RpcServer::workerThread() {
    while (running_) {
        // 简化的工作线程实现
        // 实际实现中需要从传输层接收请求并处理
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::string RpcServer::handleRequest(const std::string& request_data) {
    auto start_time = std::chrono::steady_clock::now();
    stats_.total_requests++;
    
    try {
        RpcRequest request;
        if (!serializer_->deserialize(request_data, request)) {
            stats_.failed_requests++;
            RpcResponse error_response;
            error_response.error_code = ErrorCode::SERIALIZATION_ERROR;
            error_response.error_message = "Failed to deserialize request";
            return serializer_->serialize(error_response);
        }
        
        CallContext context;
        context.request_id = request.id;
        context.start_time = start_time;
        
        RpcResponse response = processRequest(request, context);
        
        if (response.isSuccess()) {
            stats_.successful_requests++;
        } else {
            stats_.failed_requests++;
        }
        
        updateStatistics(start_time, response.isSuccess());
        return serializer_->serialize(response);
        
    } catch (const std::exception& e) {
        stats_.failed_requests++;
        RpcResponse error_response;
        error_response.error_code = ErrorCode::INTERNAL_ERROR;
        error_response.error_message = e.what();
        updateStatistics(start_time, false);
        return serializer_->serialize(error_response);
    }
}

RpcResponse RpcServer::processRequest(const RpcRequest& request, const CallContext& /* context */) {
    RpcResponse response;
    response.id = request.id;
    
    std::lock_guard<std::mutex> lock(methods_mutex_);
    
    // 首先检查高级方法处理器
    auto advanced_it = advanced_methods_.find(request.method);
    if (advanced_it != advanced_methods_.end()) {
        try {
            CallContext ctx;
            ctx.request_id = request.id;
            ctx.start_time = std::chrono::steady_clock::now();
            response.result = advanced_it->second(request.params, ctx);
            response.error_code = ErrorCode::SUCCESS;
        } catch (const std::exception& e) {
            response.error_code = ErrorCode::INTERNAL_ERROR;
            response.error_message = e.what();
        }
        return response;
    }
    
    // 检查普通方法处理器
    auto it = methods_.find(request.method);
    if (it == methods_.end()) {
        response.error_code = ErrorCode::METHOD_NOT_FOUND;
        response.error_message = "Method not found: " + request.method;
        return response;
    }
    
    try {
        response.result = it->second(request.params);
        response.error_code = ErrorCode::SUCCESS;
    } catch (const std::exception& e) {
        response.error_code = ErrorCode::INTERNAL_ERROR;
        response.error_message = e.what();
    }
    
    return response;
}

bool RpcServer::runMiddlewares(const RpcRequest& request, RpcResponse& response, const CallContext& context) {
    std::lock_guard<std::mutex> lock(middlewares_mutex_);
    for (auto& middleware : middlewares_) {
        if (!middleware(request, response, context)) {
            return false;
        }
    }
    return true;
}

void RpcServer::updateStatistics(const std::chrono::steady_clock::time_point& start_time, bool /* success */) {
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 简化的平均响应时间计算
    uint64_t current_avg = stats_.avg_response_time_ms.load();
    uint64_t new_avg = (current_avg + duration.count()) / 2;
    stats_.avg_response_time_ms = new_avg;
}

// MemoryServiceDiscovery 实现
bool MemoryServiceDiscovery::registerService(const std::string& service_name, 
                                            const ServiceEndpoint& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    services_[service_name].push_back(endpoint);
    notifyCallbacks(service_name);
    return true;
}

bool MemoryServiceDiscovery::unregisterService(const std::string& service_name,
                                              const ServiceEndpoint& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = services_.find(service_name);
    if (it != services_.end()) {
        auto& endpoints = it->second;
        endpoints.erase(
            std::remove_if(endpoints.begin(), endpoints.end(),
                [&endpoint](const ServiceEndpoint& ep) {
                    return ep.host == endpoint.host && ep.port == endpoint.port;
                }),
            endpoints.end());
        notifyCallbacks(service_name);
        return true;
    }
    return false;
}

std::vector<ServiceEndpoint> MemoryServiceDiscovery::discoverService(const std::string& service_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = services_.find(service_name);
    if (it != services_.end()) {
        return it->second;
    }
    return {};
}

void MemoryServiceDiscovery::setDiscoveryCallback(const std::string& service_name,
                                                 ServiceDiscoveryCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_[service_name] = callback;
}

void MemoryServiceDiscovery::notifyCallbacks(const std::string& service_name) {
    auto callback_it = callbacks_.find(service_name);
    if (callback_it != callbacks_.end()) {
        auto service_it = services_.find(service_name);
        if (service_it != services_.end()) {
            callback_it->second(service_it->second);
        }
    }
}

// RpcServerCluster 实现
RpcServerCluster::RpcServerCluster(std::shared_ptr<ServiceDiscovery> discovery)
    : discovery_(discovery) {}

RpcServerCluster::~RpcServerCluster() {
    stopAll();
}

void RpcServerCluster::addServer(const std::string& service_name, 
                                std::unique_ptr<RpcServer> server,
                                const ServiceEndpoint& endpoint) {
    std::lock_guard<std::mutex> lock(servers_mutex_);
    ServerInfo info;
    info.service_name = service_name;
    info.server = std::move(server);
    info.endpoint = endpoint;
    info.running = false;
    servers_.push_back(std::move(info));
}

void RpcServerCluster::removeServer(const std::string& service_name) {
    std::lock_guard<std::mutex> lock(servers_mutex_);
    servers_.erase(
        std::remove_if(servers_.begin(), servers_.end(),
            [&service_name](const ServerInfo& info) {
                return info.service_name == service_name;
            }),
        servers_.end());
}

bool RpcServerCluster::startAll() {
    std::lock_guard<std::mutex> lock(servers_mutex_);
    bool all_started = true;
    
    for (auto& info : servers_) {
        if (!info.running && info.server) {
            if (info.server->start(info.endpoint)) {
                info.running = true;
                if (discovery_) {
                    discovery_->registerService(info.service_name, info.endpoint);
                }
            } else {
                all_started = false;
            }
        }
    }
    
    return all_started;
}

void RpcServerCluster::stopAll() {
    std::lock_guard<std::mutex> lock(servers_mutex_);
    
    for (auto& info : servers_) {
        if (info.running && info.server) {
            info.server->stop();
            info.running = false;
            if (discovery_) {
                discovery_->unregisterService(info.service_name, info.endpoint);
            }
        }
    }
}

RpcServerCluster::ClusterStatus RpcServerCluster::getStatus() const {
    std::lock_guard<std::mutex> lock(servers_mutex_);
    ClusterStatus status;
    status.total_servers = servers_.size();
    status.running_servers = 0;
    status.stopped_servers = 0;
    
    for (const auto& info : servers_) {
        if (info.running) {
            status.running_servers++;
        } else {
            status.stopped_servers++;
        }
    }
    
    return status;
}

} // namespace rpc