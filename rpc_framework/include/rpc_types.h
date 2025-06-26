#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <chrono>

// C++17 std::any支持检查
#if __cplusplus >= 201703L
#include <any>
#define HAS_STD_ANY 1
#else
#define HAS_STD_ANY 0
#endif

namespace rpc {

// 类型擦除包装器（用于不支持std::any的编译器）
#if HAS_STD_ANY
using AnyValue = std::any;
#else
// 简化的AnyValue实现
class AnyValue {
private:
    enum Type {
        NONE,
        INT,
        DOUBLE,
        BOOL,
        STRING
    };
    
    Type type_;
    union {
        int int_val;
        double double_val;
        bool bool_val;
    };
    std::string string_val;
    
public:
    AnyValue() : type_(NONE) {}
    
    AnyValue(int value) : type_(INT), int_val(value) {}
    AnyValue(double value) : type_(DOUBLE), double_val(value) {}
    AnyValue(bool value) : type_(BOOL), bool_val(value) {}
    AnyValue(const std::string& value) : type_(STRING), string_val(value) {}
    AnyValue(const char* value) : type_(STRING), string_val(value) {}
    
    // 拷贝构造函数
    AnyValue(const AnyValue& other) : type_(other.type_) {
        switch (type_) {
            case INT: int_val = other.int_val; break;
            case DOUBLE: double_val = other.double_val; break;
            case BOOL: bool_val = other.bool_val; break;
            case STRING: string_val = other.string_val; break;
            default: break;
        }
    }
    
    // 赋值操作符
    AnyValue& operator=(const AnyValue& other) {
        if (this != &other) {
            type_ = other.type_;
            switch (type_) {
                case INT: int_val = other.int_val; break;
                case DOUBLE: double_val = other.double_val; break;
                case BOOL: bool_val = other.bool_val; break;
                case STRING: string_val = other.string_val; break;
                default: break;
            }
        }
        return *this;
    }
    
    template<typename T>
    T cast() const;
    
    bool has_value() const { return type_ != NONE; }
};

// 模板特化
template<>
inline int AnyValue::cast<int>() const {
    if (type_ != INT) throw std::runtime_error("Type mismatch: expected int");
    return int_val;
}

template<>
inline double AnyValue::cast<double>() const {
    if (type_ != DOUBLE) throw std::runtime_error("Type mismatch: expected double");
    return double_val;
}

template<>
inline bool AnyValue::cast<bool>() const {
    if (type_ != BOOL) throw std::runtime_error("Type mismatch: expected bool");
    return bool_val;
}

template<>
inline std::string AnyValue::cast<std::string>() const {
    if (type_ != STRING) throw std::runtime_error("Type mismatch: expected string");
    return string_val;
}

// 添加const std::string&的特化
template<>
inline const std::string& AnyValue::cast<const std::string&>() const {
    if (type_ != STRING) throw std::runtime_error("Type mismatch: expected string");
    return string_val;
}
#endif

// RPC协议类型
enum class ProtocolType {
    HTTP,           // HTTP协议
    TCP,            // 原生TCP协议  
    WEBSOCKET,      // WebSocket协议
    UDP             // UDP协议
};

// 序列化类型
enum class SerializationType {
    JSON,           // JSON序列化
    MESSAGEPACK,    // MessagePack二进制序列化
    PROTOBUF,       // Protocol Buffers
    BINARY          // 自定义二进制格式
};

// RPC调用类型
enum class CallType {
    SYNC,           // 同步调用
    ASYNC,          // 异步调用
    ONEWAY          // 单向调用（不需要响应）
};

// RPC错误码
enum class ErrorCode {
    SUCCESS = 0,
    INVALID_REQUEST = 1,
    METHOD_NOT_FOUND = 2,
    INVALID_PARAMS = 3,
    INTERNAL_ERROR = 4,
    TIMEOUT = 5,
    NETWORK_ERROR = 6,
    SERIALIZATION_ERROR = 7,
    AUTHENTICATION_ERROR = 8,
    AUTHORIZATION_ERROR = 9
};

// RPC请求结构
struct RpcRequest {
    std::string id;                             // 请求ID
    std::string method;                         // 方法名
    std::vector<AnyValue> params;              // 参数列表
    std::map<std::string, std::string> headers; // 请求头
    CallType call_type = CallType::SYNC;        // 调用类型
    std::chrono::milliseconds timeout{5000};    // 超时时间
    
    RpcRequest() = default;
    RpcRequest(const std::string& method_name) : method(method_name) {}
};

// RPC响应结构
struct RpcResponse {
    std::string id;                             // 对应请求ID
    AnyValue result;                           // 返回结果
    ErrorCode error_code = ErrorCode::SUCCESS;  // 错误码
    std::string error_message;                  // 错误信息
    std::map<std::string, std::string> headers; // 响应头
    
    bool isSuccess() const { return error_code == ErrorCode::SUCCESS; }
};

// RPC服务端点信息
struct ServiceEndpoint {
    std::string host;
    int port;
    ProtocolType protocol;
    SerializationType serialization;
    
    // 默认构造函数
    ServiceEndpoint() : host(""), port(0), protocol(ProtocolType::TCP), serialization(SerializationType::JSON) {}
    
    ServiceEndpoint(const std::string& h, int p, 
                   ProtocolType proto = ProtocolType::TCP,
                   SerializationType serial = SerializationType::JSON)
        : host(h), port(p), protocol(proto), serialization(serial) {}
        
    std::string toString() const {
        return host + ":" + std::to_string(port);
    }
};

// 方法处理器类型定义
using MethodHandler = std::function<AnyValue(const std::vector<AnyValue>&)>;

// 异步回调类型定义
using AsyncCallback = std::function<void(const RpcResponse&)>;

// 连接状态回调
using ConnectionCallback = std::function<void(bool connected, const std::string& endpoint)>;

// 服务发现回调
using ServiceDiscoveryCallback = std::function<void(const std::vector<ServiceEndpoint>&)>;

// 工具函数：错误码转字符串
inline std::string errorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS: return "Success";
        case ErrorCode::INVALID_REQUEST: return "Invalid Request";
        case ErrorCode::METHOD_NOT_FOUND: return "Method Not Found";
        case ErrorCode::INVALID_PARAMS: return "Invalid Parameters";
        case ErrorCode::INTERNAL_ERROR: return "Internal Error";
        case ErrorCode::TIMEOUT: return "Timeout";
        case ErrorCode::NETWORK_ERROR: return "Network Error";
        case ErrorCode::SERIALIZATION_ERROR: return "Serialization Error";
        case ErrorCode::AUTHENTICATION_ERROR: return "Authentication Error";
        case ErrorCode::AUTHORIZATION_ERROR: return "Authorization Error";
        default: return "Unknown Error";
    }
}

// 工具函数：协议类型转字符串
inline std::string protocolTypeToString(ProtocolType type) {
    switch (type) {
        case ProtocolType::HTTP: return "HTTP";
        case ProtocolType::TCP: return "TCP";
        case ProtocolType::WEBSOCKET: return "WebSocket";
        case ProtocolType::UDP: return "UDP";
        default: return "Unknown";
    }
}

// 工具函数：序列化类型转字符串
inline std::string serializationTypeToString(SerializationType type) {
    switch (type) {
        case SerializationType::JSON: return "JSON";
        case SerializationType::MESSAGEPACK: return "MessagePack";
        case SerializationType::PROTOBUF: return "Protobuf";
        case SerializationType::BINARY: return "Binary";
        default: return "Unknown";
    }
}

} // namespace rpc 