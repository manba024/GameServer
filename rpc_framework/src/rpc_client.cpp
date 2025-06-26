#include "../include/rpc_client.h"
#include "../include/transport.h"
#include "../include/serializer.h"
#include <iostream>
#include <sstream>
#include <random>
#include <future>

namespace rpc {

RpcClient::RpcClient(ProtocolType protocol, SerializationType serialization)
    : current_endpoint_("", 0), timeout_(std::chrono::milliseconds(5000)), 
      next_request_id_(0), running_(false) {
    
    // 创建传输层
    switch (protocol) {
        case ProtocolType::TCP:
            transport_ = std::make_unique<TcpTransport>();
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

RpcClient::~RpcClient() {
    disconnect();
}

bool RpcClient::connect(const ServiceEndpoint& endpoint) {
    current_endpoint_ = endpoint;
    bool success = transport_->connect(endpoint);
    
    return success;
}

void RpcClient::disconnect() {
    if (transport_) {
        transport_->disconnect();
    }
}

bool RpcClient::isConnected() const {
    return transport_ && transport_->isConnected();
}

RpcResponse RpcClient::call(const std::string& method, const std::vector<AnyValue>& params) {
    if (!isConnected()) {
        RpcResponse error_response;
        error_response.error_code = ErrorCode::NETWORK_ERROR;
        error_response.error_message = "Not connected to server";
        stats_.failed_requests++;
        return error_response;
    }
    
    // 记录请求开始时间
    stats_.total_requests++;
    
    try {
        // 创建请求
        RpcRequest request;
        request.id = generateRequestId();
        request.method = method;
        request.params = params;
        request.call_type = CallType::SYNC;
        request.timeout = timeout_;
        
        // 序列化请求
        std::string request_data = serializer_->serialize(request);
        
        // 发送请求
        if (!transport_->send(request_data)) {
            stats_.failed_requests++;
            RpcResponse error_response;
            error_response.error_code = ErrorCode::NETWORK_ERROR;
            error_response.error_message = "Failed to send request";
            return error_response;
        }
        
        // 接收响应
        std::string response_data = transport_->receive();
        if (response_data.empty()) {
            stats_.timeout_requests++;
            RpcResponse error_response;
            error_response.error_code = ErrorCode::TIMEOUT;
            error_response.error_message = "Request timeout";
            return error_response;
        }
        
        // 反序列化响应
        RpcResponse response;
        if (!serializer_->deserialize(response_data, response)) {
            stats_.failed_requests++;
            RpcResponse error_response;
            error_response.error_code = ErrorCode::SERIALIZATION_ERROR;
            error_response.error_message = "Failed to deserialize response";
            return error_response;
        }
        
        if (response.isSuccess()) {
            stats_.successful_requests++;
        } else {
            stats_.failed_requests++;
        }
        
        return response;
        
    } catch (const std::exception& e) {
        stats_.failed_requests++;
        RpcResponse error_response;
        error_response.error_code = ErrorCode::INTERNAL_ERROR;
        error_response.error_message = e.what();
        return error_response;
    }
}

void RpcClient::callAsync(const std::string& method, const std::vector<AnyValue>& params, AsyncCallback callback) {
    // 在新线程中执行异步调用
    std::thread([this, method, params, callback]() {
        RpcResponse response = call(method, params);
        if (callback) {
            callback(response);
        }
    }).detach();
}

std::future<RpcResponse> RpcClient::callAsync(const std::string& method, const std::vector<AnyValue>& params) {
    // 创建promise/future对
    auto promise = std::make_shared<std::promise<RpcResponse>>();
    auto future = promise->get_future();
    
    // 在新线程中执行异步调用
    std::thread([this, method, params, promise]() {
        try {
            RpcResponse response = call(method, params);
            promise->set_value(response);
        } catch (const std::exception& e) {
            RpcResponse error_response;
            error_response.error_code = ErrorCode::INTERNAL_ERROR;
            error_response.error_message = e.what();
            promise->set_value(error_response);
        }
    }).detach();
    
    return future;
}

bool RpcClient::callOneWay(const std::string& method, const std::vector<AnyValue>& params) {
    if (!isConnected()) {
        stats_.failed_requests++;
        return false;
    }
    
    try {
        // 创建单向请求
        RpcRequest request;
        request.id = generateRequestId();
        request.method = method;
        request.params = params;
        request.call_type = CallType::ONEWAY;
        
        // 序列化并发送请求
        std::string request_data = serializer_->serialize(request);
        bool success = transport_->send(request_data);
        
        if (success) {
            stats_.total_requests++;
        } else {
            stats_.failed_requests++;
        }
        
        return success;
        
    } catch (const std::exception& e) {
        stats_.failed_requests++;
        return false;
    }
}

void RpcClient::setTimeout(std::chrono::milliseconds timeout) {
    timeout_ = timeout;
    if (transport_) {
        transport_->setTimeout(timeout);
    }
}

void RpcClient::setConnectionCallback(ConnectionCallback callback) {
    if (transport_) {
        transport_->setConnectionCallback(callback);
    }
}

void RpcClient::resetStatistics() {
    stats_.total_requests = 0;
    stats_.successful_requests = 0;
    stats_.failed_requests = 0;
    stats_.timeout_requests = 0;
    stats_.bytes_sent = 0;
    stats_.bytes_received = 0;
}

std::string RpcClient::generateRequestId() {
    std::ostringstream oss;
    oss << "req_" << ++next_request_id_ << "_" << std::chrono::steady_clock::now().time_since_epoch().count();
    return oss.str();
}

} // namespace rpc