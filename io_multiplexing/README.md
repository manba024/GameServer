# IO复用框架 (IO Multiplexing Framework)

## 概述

这是一个基于工厂模式设计的IO复用框架，支持多种IO复用模型（Select、Poll、Epoll、Kqueue），让你能够轻松地在不同平台上使用最适合的IO复用技术。

## 什么是IO复用？

IO复用是一种让单线程能够同时监控多个文件描述符（如socket）状态的技术。当某个文件描述符准备好进行IO操作时，系统会通知应用程序，这样就可以避免阻塞等待，提高程序效率。

### 传统IO模型的问题

```cpp
// 传统阻塞IO - 每个连接需要一个线程
while (true) {
    int clientFd = accept(serverFd, ...); // 阻塞等待连接
    std::thread([clientFd]() {
        char buffer[1024];
        recv(clientFd, buffer, sizeof(buffer), 0); // 阻塞等待数据
        // 处理数据...
    }).detach();
}
```

### IO复用的优势

```cpp
// IO复用 - 一个线程处理多个连接
auto multiplexer = createBestMultiplexer();
multiplexer->addFd(serverFd, IOEventType::Read);

multiplexer->setEventCallback([](const IOEvent& event) {
    if (event.fd == serverFd) {
        // 有新连接
        int clientFd = accept(serverFd, ...);
        multiplexer->addFd(clientFd, IOEventType::Read);
    } else {
        // 客户端有数据可读
        char buffer[1024];
        recv(event.fd, buffer, sizeof(buffer), 0);
        // 处理数据...
    }
});

multiplexer->run(); // 事件循环
```

## 支持的IO复用模型

### 1. Select
- **优点**: 兼容性最好，所有POSIX系统都支持
- **缺点**: 有文件描述符数量限制（通常1024），性能较差
- **适用场景**: 连接数较少的简单应用

### 2. Poll
- **优点**: 没有文件描述符数量限制，API更清晰
- **缺点**: 性能仍然不够理想
- **适用场景**: 中等规模的应用

### 3. Epoll (Linux)
- **优点**: 高性能，支持大量连接，支持边缘触发
- **缺点**: 只在Linux上可用
- **适用场景**: Linux上的高并发服务器

### 4. Kqueue (BSD/macOS)
- **优点**: 高性能，功能丰富，支持多种事件类型
- **缺点**: 只在BSD系统和macOS上可用
- **适用场景**: BSD/macOS上的高性能应用

## 架构设计

```
IOMultiplexer (抽象基类)
    ├── SelectMultiplexer
    ├── PollMultiplexer
    ├── EpollMultiplexer (Linux only)
    └── KqueueMultiplexer (BSD/macOS only)

MultiplexerFactory (工厂类)
    ├── create(type) - 创建指定类型
    ├── createBest() - 创建最适合当前平台的
    └── getSupportedTypes() - 获取支持的类型
```

## 项目结构

```
io_multiplexing/
├── include/                    # 头文件目录
│   ├── io_multiplexer.h       # 抽象基类和接口定义
│   ├── select_multiplexer.h   # Select实现头文件
│   ├── poll_multiplexer.h     # Poll实现头文件
│   ├── epoll_multiplexer.h    # Epoll实现头文件（Linux）
│   ├── kqueue_multiplexer.h   # Kqueue实现头文件（BSD/macOS）
│   └── multiplexer_factory.h  # 工厂类头文件
├── src/                        # 源文件目录
│   ├── select_multiplexer.cpp  # Select实现
│   ├── poll_multiplexer.cpp    # Poll实现
│   ├── epoll_multiplexer.cpp   # Epoll实现（Linux）
│   ├── kqueue_multiplexer.cpp  # Kqueue实现（BSD/macOS）
│   └── multiplexer_factory.cpp # 工厂实现
├── examples/                   # 示例程序目录
│   └── demo_basic.cpp         # 基础演示程序
├── CMakeLists.txt             # 构建配置文件
└── README.md                  # 说明文档
```

## 快速开始

### 1. 编译

```bash
cd io_multiplexing
mkdir build && cd build
cmake ..
make
```

编译成功后，你会看到类似这样的输出：
```
-- 添加Kqueue支持 (macOS/BSD)  # 在macOS上
-- 编译源文件:
--   src/select_multiplexer.cpp
--   src/poll_multiplexer.cpp
--   src/multiplexer_factory.cpp
--   src/kqueue_multiplexer.cpp
```

### 2. 运行演示

```bash
# 运行基础演示程序
./bin/demo_basic
```

### 3. 测试网络功能

演示程序会启动一个回显服务器，你可以使用telnet测试：

```bash
# 在另一个终端中
telnet localhost 8080
```

然后输入任何文本，服务器会回显你的输入。

## API参考

### IOMultiplexer 接口

```cpp
class IOMultiplexer {
public:
    // 添加文件描述符监听
    virtual bool addFd(int fd, uint32_t events, void* userData = nullptr) = 0;
    
    // 修改文件描述符的监听事件
    virtual bool modifyFd(int fd, uint32_t events, void* userData = nullptr) = 0;
    
    // 移除文件描述符
    virtual bool removeFd(int fd) = 0;
    
    // 等待事件（单次）
    virtual std::vector<IOEvent> wait(int timeout = -1) = 0;
    
    // 设置事件回调函数
    virtual void setEventCallback(EventCallback callback);
    
    // 运行事件循环
    virtual void run() = 0;
    
    // 停止事件循环
    virtual void stop() = 0;
    
    // 获取类型名称
    virtual std::string getTypeName() const = 0;
    
    // 获取当前监听的fd数量
    virtual size_t getFdCount() const = 0;
    
    // 获取最大支持的fd数量
    virtual size_t getMaxFdCount() const = 0;
};
```

### 事件类型

```cpp
enum class IOEventType {
    Read = 1,      // 可读事件
    Write = 2,     // 可写事件
    Error = 4,     // 错误事件
    HangUp = 8     // 连接挂起
};
```

### 工厂方法

```cpp
// 创建指定类型的IO复用器
auto multiplexer = createMultiplexer(MultiplexerType::Epoll);

// 创建最适合当前平台的IO复用器
auto multiplexer = createBestMultiplexer();

// 获取支持的类型
auto types = MultiplexerFactory::getSupportedTypes();

// 检查是否支持某种类型
bool supported = MultiplexerFactory::isSupported(MultiplexerType::Epoll);
```

## 使用示例

### 1. 简单的回显服务器

```cpp
#include "multiplexer_factory.h"
using namespace IOMultiplexing;

auto multiplexer = createBestMultiplexer();
int serverFd = createServerSocket(8080);

// 监听服务器socket
multiplexer->addFd(serverFd, static_cast<uint32_t>(IOEventType::Read));

multiplexer->setEventCallback([&](const IOEvent& event) {
    if (event.fd == serverFd) {
        // 新连接
        int clientFd = accept(serverFd, nullptr, nullptr);
        multiplexer->addFd(clientFd, static_cast<uint32_t>(IOEventType::Read));
    } else {
        // 客户端数据
        char buffer[1024];
        int len = recv(event.fd, buffer, sizeof(buffer), 0);
        if (len > 0) {
            send(event.fd, buffer, len, 0); // 回显
        } else {
            multiplexer->removeFd(event.fd);
            close(event.fd);
        }
    }
});

multiplexer->run();
```

### 2. 使用用户数据

```cpp
struct ClientInfo {
    std::string name;
    time_t connectTime;
};

multiplexer->setEventCallback([](const IOEvent& event) {
    ClientInfo* info = static_cast<ClientInfo*>(event.userData);
    if (info) {
        std::cout << "客户端 " << info->name << " 有事件发生\n";
    }
});

// 添加fd时传入用户数据
ClientInfo* info = new ClientInfo{"客户端1", time(nullptr)};
multiplexer->addFd(clientFd, static_cast<uint32_t>(IOEventType::Read), info);
```

### 3. 平台特定的优化

```cpp
// 根据平台选择最优的IO复用器
auto multiplexer = createBestMultiplexer();

std::cout << "使用 " << multiplexer->getTypeName() << " IO复用器\n";
std::cout << "最大支持fd数: " << multiplexer->getMaxFdCount() << "\n";

// 在Linux上会自动选择Epoll
// 在macOS上会自动选择Kqueue
// 在其他平台上会选择Poll或Select
```

## 性能对比

| 模型 | 连接数 | CPU使用率 | 内存使用 | 延迟 | 特点 |
|------|--------|-----------|----------|------|------|
| Select | < 1000 | 高 | 低 | 中等 | 兼容性最好 |
| Poll | < 10000 | 中等 | 中等 | 中等 | 无fd限制 |
| Epoll | > 100000 | 低 | 低 | 低 | Linux高性能 |
| Kqueue | > 100000 | 低 | 低 | 低 | BSD/macOS高性能 |

## 最佳实践

### 1. 选择合适的模型

```cpp
// ✅ 推荐：让框架自动选择
auto multiplexer = createBestMultiplexer();

// ✅ 或者根据需求手动选择
#ifdef __linux__
    auto multiplexer = createMultiplexer(MultiplexerType::Epoll);
#else
    auto multiplexer = createMultiplexer(MultiplexerType::Poll);
#endif
```

### 2. 事件处理

```cpp
// ✅ 好的做法：快速处理事件
multiplexer->setEventCallback([](const IOEvent& event) {
    if (event.events & static_cast<uint32_t>(IOEventType::Read)) {
        // 快速读取数据并处理
        handleReadEvent(event.fd);
    }
    if (event.events & static_cast<uint32_t>(IOEventType::Error)) {
        // 处理错误
        handleError(event.fd);
    }
});

// ❌ 避免的做法：在回调中执行耗时操作
multiplexer->setEventCallback([](const IOEvent& event) {
    // 不要在这里执行耗时操作！
    heavyComputation(); // 这会阻塞其他事件
});
```

### 3. 资源管理

```cpp
// ✅ 正确的资源清理
multiplexer->setEventCallback([&](const IOEvent& event) {
    if (event.events & static_cast<uint32_t>(IOEventType::Error) ||
        event.events & static_cast<uint32_t>(IOEventType::HangUp)) {
        
        // 清理资源
        multiplexer->removeFd(event.fd);
        close(event.fd);
        
        // 清理用户数据
        if (event.userData) {
            delete static_cast<ClientInfo*>(event.userData);
        }
    }
});
```

### 4. 错误处理

```cpp
// ✅ 完整的错误处理
auto multiplexer = createBestMultiplexer();
if (!multiplexer) {
    std::cerr << "创建IO复用器失败" << std::endl;
    return -1;
}

if (!multiplexer->addFd(serverFd, static_cast<uint32_t>(IOEventType::Read))) {
    std::cerr << "添加服务器fd失败" << std::endl;
    return -1;
}
```

## 高级特性

### 1. Epoll边缘触发模式

```cpp
// Epoll实现默认使用边缘触发模式（ET）
// 这意味着你需要一次性读取所有可用数据
void handleReadEvent(int fd) {
    char buffer[4096];
    while (true) {
        ssize_t n = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if (n > 0) {
            // 处理数据
            processData(buffer, n);
        } else if (n == 0) {
            // 连接关闭
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 没有更多数据了
                break;
            }
            // 其他错误
            break;
        }
    }
}
```

### 2. Kqueue高级事件

```cpp
// Kqueue支持更多类型的事件
// 当前实现主要支持读写事件，但可以扩展支持：
// - 文件系统事件
// - 信号事件
// - 定时器事件
// - 进程事件
```

## 扩展和定制

### 1. 自定义IO复用器

```cpp
class CustomMultiplexer : public IOMultiplexer {
public:
    bool addFd(int fd, uint32_t events, void* userData) override {
        // 自定义实现
        return true;
    }
    
    std::vector<IOEvent> wait(int timeout) override {
        // 自定义等待逻辑
        return {};
    }
    
    // 实现其他虚函数...
};
```

### 2. 扩展工厂

```cpp
// 在工厂中添加新的类型
enum class MultiplexerType {
    Select,
    Poll,
    Epoll,
    Kqueue,
    Custom  // 新增类型
};
```

## 注意事项

### 1. 线程安全
- 当前实现**不是线程安全的**
- 如需多线程使用，请在外部加锁
- 建议每个线程使用独立的IO复用器实例

### 2. 文件描述符管理
- 确保文件描述符在使用前是有效的
- 记得在不需要时关闭文件描述符
- 注意系统的文件描述符限制（`ulimit -n`）

### 3. 平台兼容性
- Epoll只在Linux上可用
- Kqueue只在BSD/macOS上可用
- Select和Poll在所有POSIX系统上都可用

### 4. 内存管理
- 使用`userData`时要注意内存管理
- 确保在删除fd时清理相关的用户数据
- 避免内存泄漏

## 故障排除

### 编译问题

**问题**: 找不到头文件
```bash
fatal error: sys/epoll.h: No such file or directory
```
**解决**: 这是正常的，在非Linux系统上epoll不可用。CMake会自动处理平台差异。

**问题**: C++11支持
```bash
error: 'auto' type specifier is a C++11 extension
```
**解决**: 确保使用支持C++11的编译器，或者更新CMake配置。

### 运行时问题

**问题**: 端口被占用
```bash
bind: Address already in use
```
**解决**: 更换端口或等待端口释放。

**问题**: 权限不足
```bash
bind: Permission denied
```
**解决**: 使用大于1024的端口，或者以管理员权限运行。

**问题**: 文件描述符耗尽
```bash
accept: Too many open files
```
**解决**: 检查系统限制（`ulimit -n`），或者优化程序的fd使用。

### 性能问题

**问题**: CPU使用率过高
- 检查是否在事件回调中执行了耗时操作
- 考虑使用更高效的IO复用模型（Epoll/Kqueue）
- 优化事件处理逻辑

**问题**: 内存使用过多
- 检查是否有内存泄漏
- 确保正确清理用户数据
- 考虑使用对象池

## 学习资源

### 书籍推荐
1. **《Unix网络编程》** - W. Richard Stevens
   - 第6章：I/O复用
   - 经典教材，详细讲解各种IO模型

2. **《Linux高性能服务器编程》** - 游双
   - 第9章：I/O复用
   - 实战导向，包含大量代码示例

### 在线资源
1. **Linux内核文档** - epoll相关
   - https://www.kernel.org/doc/html/latest/filesystems/epoll.html

2. **FreeBSD手册** - kqueue详细说明
   - https://www.freebsd.org/cgi/man.cgi?query=kqueue

3. **本项目示例代码** - 最佳实践参考
   - 查看`examples/`目录下的示例程序

### 相关技术
- **Reactor模式**: 事件驱动的设计模式
- **Proactor模式**: 异步IO的设计模式  
- **事件循环**: 主循环的设计思想
- **非阻塞IO**: 提高并发性能的关键技术

## 贡献指南

欢迎贡献代码！请遵循以下步骤：

1. Fork本项目
2. 创建特性分支
3. 提交更改
4. 发起Pull Request

### 代码规范
- 使用4个空格缩进
- 函数和变量使用驼峰命名
- 添加必要的注释
- 保持代码简洁清晰

### 测试
- 在多个平台上测试
- 添加单元测试
- 确保向后兼容性

现在你已经有了一个完整的IO复用框架！这个框架不仅包含了所有四种主要的IO复用模型实现，还提供了工厂模式的统一接口，让你可以轻松地在不同平台上使用最适合的IO复用技术。 