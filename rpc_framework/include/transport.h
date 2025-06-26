#pragma once

#include "rpc_types.h"
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <map>

namespace rpc {

// 传输层抽象基类
class Transport {
public:
    virtual ~Transport() = default;
    
    // 发送数据
    virtual bool send(const std::string& data) = 0;
    
    // 接收数据（阻塞）
    virtual std::string receive() = 0;
    
    // 连接到服务器
    virtual bool connect(const ServiceEndpoint& endpoint) = 0;
    
    // 断开连接
    virtual void disconnect() = 0;
    
    // 检查连接状态
    virtual bool isConnected() const = 0;
    
    // 获取协议类型
    virtual ProtocolType getProtocolType() const = 0;
    
    // 设置超时时间
    virtual void setTimeout(std::chrono::milliseconds timeout) = 0;
    
    // 设置连接状态回调
    virtual void setConnectionCallback(ConnectionCallback callback) = 0;
};

// 服务器传输层接口
class ServerTransport {
public:
    virtual ~ServerTransport() = default;
    
    // 启动服务器
    virtual bool start(const ServiceEndpoint& endpoint) = 0;
    
    // 停止服务器
    virtual void stop() = 0;
    
    // 检查服务器状态
    virtual bool isRunning() const = 0;
    
    // 设置消息处理回调
    virtual void setMessageHandler(std::function<std::string(const std::string&)> handler) = 0;
    
    // 获取协议类型
    virtual ProtocolType getProtocolType() const = 0;
};

// TCP传输实现
class TcpTransport : public Transport {
public:
    TcpTransport();
    ~TcpTransport() override;
    
    bool send(const std::string& data) override;
    std::string receive() override;
    bool connect(const ServiceEndpoint& endpoint) override;
    void disconnect() override;
    bool isConnected() const override;
    ProtocolType getProtocolType() const override { return ProtocolType::TCP; }
    void setTimeout(std::chrono::milliseconds timeout) override;
    void setConnectionCallback(ConnectionCallback callback) override;

private:
    int socket_fd_;
    std::atomic<bool> connected_;
    std::chrono::milliseconds timeout_;
    ConnectionCallback connection_callback_;
    mutable std::mutex mutex_;
    
    bool setSocketNonBlocking(int fd);
    bool waitForSocket(int fd, bool for_read, std::chrono::milliseconds timeout);
    bool sendAll(const char* data, size_t length);
    bool receiveAll(char* buffer, size_t length);
};

// TCP服务器传输实现
class TcpServerTransport : public ServerTransport {
public:
    TcpServerTransport();
    ~TcpServerTransport() override;
    
    bool start(const ServiceEndpoint& endpoint) override;
    void stop() override;
    bool isRunning() const override;
    void setMessageHandler(std::function<std::string(const std::string&)> handler) override;
    ProtocolType getProtocolType() const override { return ProtocolType::TCP; }

private:
    int server_fd_;
    std::atomic<bool> running_;
    std::thread accept_thread_;
    std::function<std::string(const std::string&)> message_handler_;
    std::vector<std::thread> worker_threads_;
    std::mutex workers_mutex_;
    
    void acceptLoop();
    void handleClient(int client_fd);
    std::string receiveMessage(int fd);
    bool sendMessage(int fd, const std::string& message);
};

// HTTP传输实现
class HttpTransport : public Transport {
public:
    HttpTransport();
    ~HttpTransport() override;
    
    bool send(const std::string& data) override;
    std::string receive() override;
    bool connect(const ServiceEndpoint& endpoint) override;
    void disconnect() override;
    bool isConnected() const override;
    ProtocolType getProtocolType() const override { return ProtocolType::HTTP; }
    void setTimeout(std::chrono::milliseconds timeout) override;
    void setConnectionCallback(ConnectionCallback callback) override;
    
    // HTTP特有方法
    void setPath(const std::string& path) { http_path_ = path; }
    void setHeaders(const std::map<std::string, std::string>& headers) { http_headers_ = headers; }

private:
    std::unique_ptr<TcpTransport> tcp_transport_;
    std::string http_path_;
    std::map<std::string, std::string> http_headers_;
    ServiceEndpoint current_endpoint_;
    
    std::string buildHttpRequest(const std::string& body);
    std::string parseHttpResponse(const std::string& response);
};

// HTTP服务器传输实现
class HttpServerTransport : public ServerTransport {
public:
    HttpServerTransport();
    ~HttpServerTransport() override;
    
    bool start(const ServiceEndpoint& endpoint) override;
    void stop() override;
    bool isRunning() const override;
    void setMessageHandler(std::function<std::string(const std::string&)> handler) override;
    ProtocolType getProtocolType() const override { return ProtocolType::HTTP; }
    
    // HTTP特有方法
    void setPath(const std::string& path) { http_path_ = path; }

private:
    std::unique_ptr<TcpServerTransport> tcp_server_;
    std::string http_path_;
    
    std::string handleHttpRequest(const std::string& request);
    std::string parseHttpRequest(const std::string& request);
    std::string buildHttpResponse(const std::string& body, int status_code = 200);
};

// UDP传输实现
class UdpTransport : public Transport {
public:
    UdpTransport();
    ~UdpTransport() override;
    
    bool send(const std::string& data) override;
    std::string receive() override;
    bool connect(const ServiceEndpoint& endpoint) override;
    void disconnect() override;
    bool isConnected() const override;
    ProtocolType getProtocolType() const override { return ProtocolType::UDP; }
    void setTimeout(std::chrono::milliseconds timeout) override;
    void setConnectionCallback(ConnectionCallback callback) override;

private:
    int socket_fd_;
    ServiceEndpoint target_endpoint_;
    std::atomic<bool> connected_;
    std::chrono::milliseconds timeout_;
    ConnectionCallback connection_callback_;
    mutable std::mutex mutex_;
};

// 传输层工厂
class TransportFactory {
public:
    // 创建客户端传输
    static std::unique_ptr<Transport> createClient(ProtocolType type);
    
    // 创建服务器传输
    static std::unique_ptr<ServerTransport> createServer(ProtocolType type);
    
    // 获取支持的协议类型
    static std::vector<ProtocolType> getSupportedProtocols();
    
    // 检查是否支持指定协议
    static bool isSupported(ProtocolType type);
};

} // namespace rpc 