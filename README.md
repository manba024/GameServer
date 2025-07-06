# GameServer 游戏服务器框架

一个高性能、模块化的游戏服务器开发框架，提供了丰富的基础组件和工具。

## 🚀 核心特性

### 基础组件
- **IO复用框架** - 支持Select/Poll/Epoll/Kqueue的跨平台IO复用
- **线程池** - 提供固定、缓存、优先级等多种线程池实现
- **排序算法库** - 包含多种排序算法的高性能实现
- **定时器组件** - 高精度定时器和定时任务管理
- **字符串工具** - 常用字符串处理和格式化工具

### 网络和通信
- **RPC框架** - 高性能RPC框架，支持TCP/HTTP/UDP + JSON/Binary/MessagePack
- **Web服务器** - 高性能Web服务器，支持HTTP/1.1、静态文件服务、用户认证等
- **协程库** - 基于C++11的协程库，支持协程调度、同步原语和通信机制

### 应用组件
- **排行榜系统** - 支持多种数据结构的排行榜实现
- **分布式ID生成器** - 高性能的分布式唯一ID生成

## 📦 编译和安装

### 系统要求
- C++11 或更高版本
- CMake 3.12 或更高版本
- GCC 4.8+ 或 Clang 3.4+

### 编译选项
```bash
# 完整编译（默认）
cmake -B build
make -C build

# 选择性编译组件
cmake -B build -DBUILD_WEBSERVER=ON -DBUILD_COROUTINE=ON
make -C build

# 禁用特定组件
cmake -B build -DBUILD_WEBSERVER=OFF -DBUILD_COROUTINE=OFF
make -C build
```

### 可用的编译选项
- `BUILD_WEBSERVER` - 编译Web服务器组件（默认: ON）
- `BUILD_COROUTINE` - 编译协程库组件（默认: ON）
- `BUILD_TESTS` - 编译单元测试（默认: OFF）
- `BUILD_EXAMPLES` - 编译示例程序（默认: ON）
- `BUILD_BENCHMARKS` - 编译性能测试（默认: OFF）

## 🎯 快速开始

### Web服务器
```bash
# 编译并运行Web服务器
make -C build webserver
./build/bin/webserver
```

### 协程库
```bash
# 编译协程库示例
make -C build coroutine_basic_example
./build/bin/coroutine_basic_example

# 运行高级协程示例
make -C build coroutine_advanced_example
./build/bin/coroutine_advanced_example
```

### RPC框架
```bash
# 编译RPC示例
make -C build calculator_demo
./build/bin/calculator_demo
```

## 📚 组件文档

每个组件都有详细的README文档：
- [Web服务器](webserver/README.md) - 高性能Web服务器使用指南
- [协程库](coroutine/README.md) - 协程库API参考和使用示例
- [RPC框架](rpc_framework/README.md) - RPC框架开发指南
- [排行榜系统](leaderboard/README.md) - 排行榜系统使用说明

## 🏗️ 项目结构

```
GameServer/
├── io_multiplexing/     # IO复用框架
├── threadpool/          # 线程池库
├── sorting/            # 排序算法库
├── timer/              # 定时器组件
├── string_utils/       # 字符串工具
├── rpc_framework/      # RPC框架
├── webserver/          # Web服务器
├── coroutine/          # 协程库
├── leaderboard/        # 排行榜系统
├── distributed_id/     # 分布式ID生成器
└── CMakeLists.txt      # 主构建文件
```

## 🔧 使用示例

### 基本协程使用
```cpp
#include "coroutine/include/coroutine.h"
#include "coroutine/include/sync.h"

using namespace coroutine;

void my_coroutine(int id) {
    std::cout << "协程 " << id << " 开始执行" << std::endl;
    api::yield(); // 让出CPU
    std::cout << "协程 " << id << " 完成" << std::endl;
}

int main() {
    // 创建协程
    for (int i = 1; i <= 3; ++i) {
        api::create([i]() { my_coroutine(i); });
    }
    
    // 运行调度器
    api::run();
    return 0;
}
```

### Web服务器配置
```cpp
#include "webserver/include/webserver.h"

int main() {
    WebServer server(
        1316,           // 端口
        ET,             // 触发模式
        60000,          // 超时时间
        false,          // 优雅关闭
        3306,           // 数据库端口
        "root",         // 数据库用户
        "root",         // 数据库密码
        "webserver",    // 数据库名
        12,             // 连接池数量
        6,              // 线程数量
        true,           // 启用日志
        1,              // 日志级别
        1024            // 日志队列大小
    );
    
    server.Start();
    return 0;
}
```

## 🚀 性能特性

- **高并发**: 支持数万并发连接
- **低延迟**: 微秒级响应时间
- **高吞吐**: 优化的内存管理和IO操作
- **跨平台**: 支持Linux、macOS、Windows
- **模块化**: 按需编译，减少资源占用

## 📊 基准测试

运行性能测试：
```bash
cmake -B build -DBUILD_BENCHMARKS=ON
make -C build benchmark
./build/bin/benchmark
```

## 🤝 贡献指南

欢迎提交Issue和Pull Request来改进这个项目！

1. Fork本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建Pull Request

## 📄 许可证

本项目采用MIT许可证，详见[LICENSE](LICENSE)文件。

---

**GameServer** - 构建高性能游戏服务器的完整解决方案！
