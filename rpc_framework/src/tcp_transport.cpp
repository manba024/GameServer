#include "../include/transport.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <cstring>
#include <iostream>

namespace rpc {

// TCP客户端传输实现
TcpTransport::TcpTransport() 
    : socket_fd_(-1), connected_(false), timeout_(5000) {}

TcpTransport::~TcpTransport() {
    disconnect();
}

bool TcpTransport::connect(const ServiceEndpoint& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (connected_) {
        return true;
    }
    
    // 创建socket
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        if (connection_callback_) {
            connection_callback_(false, endpoint.toString());
        }
        return false;
    }
    
    // 设置非阻塞模式
    if (!setSocketNonBlocking(socket_fd_)) {
        close(socket_fd_);
        socket_fd_ = -1;
        if (connection_callback_) {
            connection_callback_(false, endpoint.toString());
        }
        return false;
    }
    
    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(endpoint.port);
    
    if (inet_pton(AF_INET, endpoint.host.c_str(), &server_addr.sin_addr) <= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
        if (connection_callback_) {
            connection_callback_(false, endpoint.toString());
        }
        return false;
    }
    
    // 连接到服务器
    int result = ::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (result == 0) {
        // 立即连接成功
        connected_ = true;
        if (connection_callback_) {
            connection_callback_(true, endpoint.toString());
        }
        return true;
    } else if (errno == EINPROGRESS) {
        // 连接正在进行，等待完成
        if (waitForSocket(socket_fd_, false, timeout_)) {
            // 检查连接是否成功
            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(socket_fd_, SOL_SOCKET, SO_ERROR, &error, &len) == 0 && error == 0) {
                connected_ = true;
                if (connection_callback_) {
                    connection_callback_(true, endpoint.toString());
                }
                return true;
            }
        }
    }
    
    // 连接失败
    close(socket_fd_);
    socket_fd_ = -1;
    if (connection_callback_) {
        connection_callback_(false, endpoint.toString());
    }
    return false;
}

void TcpTransport::disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
    
    if (connected_ && connection_callback_) {
        connection_callback_(false, "");
    }
    
    connected_ = false;
}

bool TcpTransport::isConnected() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connected_;
}

bool TcpTransport::send(const std::string& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!connected_ || socket_fd_ < 0) {
        return false;
    }
    
    // 发送数据长度（4字节网络字节序）
    uint32_t length = htonl(static_cast<uint32_t>(data.length()));
    if (!sendAll(reinterpret_cast<const char*>(&length), sizeof(length))) {
        return false;
    }
    
    // 发送数据内容
    return sendAll(data.c_str(), data.length());
}

std::string TcpTransport::receive() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!connected_ || socket_fd_ < 0) {
        return "";
    }
    
    // 接收数据长度
    uint32_t length;
    if (!receiveAll(reinterpret_cast<char*>(&length), sizeof(length))) {
        return "";
    }
    length = ntohl(length);
    
    // 接收数据内容
    std::string data(length, '\0');
    if (!receiveAll(&data[0], length)) {
        return "";
    }
    
    return data;
}

void TcpTransport::setTimeout(std::chrono::milliseconds timeout) {
    timeout_ = timeout;
}

void TcpTransport::setConnectionCallback(ConnectionCallback callback) {
    connection_callback_ = callback;
}

bool TcpTransport::setSocketNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

bool TcpTransport::waitForSocket(int fd, bool for_read, std::chrono::milliseconds timeout) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = for_read ? POLLIN : POLLOUT;
    pfd.revents = 0;
    
    int result = poll(&pfd, 1, static_cast<int>(timeout.count()));
    return result > 0 && (pfd.revents & (for_read ? POLLIN : POLLOUT));
}

bool TcpTransport::sendAll(const char* data, size_t length) {
    size_t sent = 0;
    while (sent < length) {
        if (!waitForSocket(socket_fd_, false, timeout_)) {
            return false;
        }
        
        ssize_t result = ::send(socket_fd_, data + sent, length - sent, MSG_NOSIGNAL);
        if (result <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            return false;
        }
        sent += result;
    }
    return true;
}

bool TcpTransport::receiveAll(char* buffer, size_t length) {
    size_t received = 0;
    while (received < length) {
        if (!waitForSocket(socket_fd_, true, timeout_)) {
            return false;
        }
        
        ssize_t result = recv(socket_fd_, buffer + received, length - received, 0);
        if (result <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            return false;
        }
        received += result;
    }
    return true;
}

// TCP服务器传输实现
TcpServerTransport::TcpServerTransport() 
    : server_fd_(-1), running_(false) {}

TcpServerTransport::~TcpServerTransport() {
    stop();
}

bool TcpServerTransport::start(const ServiceEndpoint& endpoint) {
    if (running_) {
        return true;
    }
    
    // 创建服务器socket
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        return false;
    }
    
    // 设置socket选项
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_fd_);
        server_fd_ = -1;
        return false;
    }
    
    // 绑定地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(endpoint.port);
    
    if (endpoint.host == "0.0.0.0" || endpoint.host.empty()) {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, endpoint.host.c_str(), &server_addr.sin_addr) <= 0) {
            close(server_fd_);
            server_fd_ = -1;
            return false;
        }
    }
    
    if (bind(server_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(server_fd_);
        server_fd_ = -1;
        return false;
    }
    
    // 开始监听
    if (listen(server_fd_, SOMAXCONN) < 0) {
        close(server_fd_);
        server_fd_ = -1;
        return false;
    }
    
    running_ = true;
    
    // 启动接受连接的线程
    accept_thread_ = std::thread(&TcpServerTransport::acceptLoop, this);
    
    return true;
}

void TcpServerTransport::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // 关闭服务器socket
    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }
    
    // 等待接受线程结束
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }
    
    // 等待所有工作线程结束
    {
        std::lock_guard<std::mutex> lock(workers_mutex_);
        for (auto& worker : worker_threads_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        worker_threads_.clear();
    }
}

bool TcpServerTransport::isRunning() const {
    return running_;
}

void TcpServerTransport::setMessageHandler(std::function<std::string(const std::string&)> handler) {
    message_handler_ = handler;
}

void TcpServerTransport::acceptLoop() {
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            break;
        }
        
        // 为每个客户端创建处理线程
        {
            std::lock_guard<std::mutex> lock(workers_mutex_);
            worker_threads_.emplace_back(&TcpServerTransport::handleClient, this, client_fd);
        }
    }
}

void TcpServerTransport::handleClient(int client_fd) {
    while (running_) {
        std::string request = receiveMessage(client_fd);
        if (request.empty()) {
            break;
        }
        
        if (message_handler_) {
            std::string response = message_handler_(request);
            if (!sendMessage(client_fd, response)) {
                break;
            }
        }
    }
    
    close(client_fd);
}

std::string TcpServerTransport::receiveMessage(int fd) {
    // 接收消息长度
    uint32_t length;
    if (recv(fd, &length, sizeof(length), MSG_WAITALL) != sizeof(length)) {
        return "";
    }
    length = ntohl(length);
    
    // 接收消息内容
    std::string message(length, '\0');
    if (recv(fd, &message[0], length, MSG_WAITALL) != static_cast<ssize_t>(length)) {
        return "";
    }
    
    return message;
}

bool TcpServerTransport::sendMessage(int fd, const std::string& message) {
    // 发送消息长度
    uint32_t length = htonl(static_cast<uint32_t>(message.length()));
    if (send(fd, &length, sizeof(length), MSG_NOSIGNAL) != sizeof(length)) {
        return false;
    }
    
    // 发送消息内容
    return send(fd, message.c_str(), message.length(), MSG_NOSIGNAL) == static_cast<ssize_t>(message.length());
}

} // namespace rpc 