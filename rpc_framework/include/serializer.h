#pragma once

#include "rpc_types.h"
#include <string>
#include <vector>
#include <memory>

namespace rpc {

// 序列化器抽象基类
class Serializer {
public:
    virtual ~Serializer() = default;
    
    // 序列化RPC请求
    virtual std::string serialize(const RpcRequest& request) = 0;
    
    // 反序列化RPC请求
    virtual bool deserialize(const std::string& data, RpcRequest& request) = 0;
    
    // 序列化RPC响应
    virtual std::string serialize(const RpcResponse& response) = 0;
    
    // 反序列化RPC响应
    virtual bool deserialize(const std::string& data, RpcResponse& response) = 0;
    
    // 获取序列化类型
    virtual SerializationType getType() const = 0;
    
    // 获取内容类型（用于HTTP头）
    virtual std::string getContentType() const = 0;
};

// JSON序列化器
class JsonSerializer : public Serializer {
public:
    JsonSerializer();
    ~JsonSerializer() override = default;
    
    std::string serialize(const RpcRequest& request) override;
    bool deserialize(const std::string& data, RpcRequest& request) override;
    
    std::string serialize(const RpcResponse& response) override;
    bool deserialize(const std::string& data, RpcResponse& response) override;
    
    SerializationType getType() const override { return SerializationType::JSON; }
    std::string getContentType() const override { return "application/json"; }

private:
    std::string serializeAnyValue(const AnyValue& value);
    AnyValue deserializeAnyValue(const std::string& json_str);
    std::string anyValueToJsonString(const AnyValue& value);
    AnyValue jsonStringToAnyValue(const std::string& json_str);
    
    // JSON解析辅助函数
    std::string escapeString(const std::string& str);
    std::string unescapeString(const std::string& str);
    std::string trim(const std::string& str);
    std::map<std::string, std::string> parseJson(const std::string& json);
    size_t findValueEnd(const std::string& content, size_t start);
    
    // JSON值提取函数
    std::string getString(const std::map<std::string, std::string>& json, const std::string& key);
    int getInt(const std::map<std::string, std::string>& json, const std::string& key);
    std::vector<std::string> getArray(const std::map<std::string, std::string>& json, const std::string& key);
    std::map<std::string, std::string> getObject(const std::map<std::string, std::string>& json, const std::string& key);
    bool hasKey(const std::map<std::string, std::string>& json, const std::string& key);
};

// 二进制序列化器
class BinarySerializer : public Serializer {
public:
    BinarySerializer();
    ~BinarySerializer() override = default;
    
    std::string serialize(const RpcRequest& request) override;
    bool deserialize(const std::string& data, RpcRequest& request) override;
    
    std::string serialize(const RpcResponse& response) override;
    bool deserialize(const std::string& data, RpcResponse& response) override;
    
    SerializationType getType() const override { return SerializationType::BINARY; }
    std::string getContentType() const override { return "application/octet-stream"; }

private:
    void writeString(std::vector<uint8_t>& buffer, const std::string& str);
    std::string readString(const uint8_t*& data, size_t& remaining);
    void writeInt32(std::vector<uint8_t>& buffer, int32_t value);
    int32_t readInt32(const uint8_t*& data, size_t& remaining);
    void writeInt64(std::vector<uint8_t>& buffer, int64_t value);
    int64_t readInt64(const uint8_t*& data, size_t& remaining);
};

// MessagePack序列化器（简化版本）
class MessagePackSerializer : public Serializer {
public:
    MessagePackSerializer();
    ~MessagePackSerializer() override = default;
    
    std::string serialize(const RpcRequest& request) override;
    bool deserialize(const std::string& data, RpcRequest& request) override;
    
    std::string serialize(const RpcResponse& response) override;
    bool deserialize(const std::string& data, RpcResponse& response) override;
    
    SerializationType getType() const override { return SerializationType::MESSAGEPACK; }
    std::string getContentType() const override { return "application/msgpack"; }

private:
    std::string packString(const std::string& str);
    std::string packInt(int64_t value);
    std::string packArray(size_t size);
    std::string packMap(size_t size);
};

// 序列化器工厂
class SerializerFactory {
public:
    // 创建指定类型的序列化器
    static std::unique_ptr<Serializer> create(SerializationType type);
    
    // 获取所有支持的序列化类型
    static std::vector<SerializationType> getSupportedTypes();
    
    // 检查是否支持指定类型
    static bool isSupported(SerializationType type);
    
    // 从内容类型字符串推断序列化类型
    static SerializationType fromContentType(const std::string& content_type);
};

} // namespace rpc 