# 线程池框架 (ThreadPool Framework)

一个功能完整、高性能的C++线程池框架，支持多种线程池实现和工厂模式。

## 🚀 特性

- **多种线程池实现**：固定大小、缓存、优先级、定时任务、工作窃取
- **工厂模式**：统一的创建接口，支持推荐算法
- **丰富的API**：任务提交、批量操作、有返回值任务
- **灵活配置**：线程数、队列大小、超时时间等
- **拒绝策略**：丢弃、阻塞、调用者执行等
- **统计监控**：任务数量、执行时间、线程状态
- **异常安全**：完善的异常处理机制
- **现代C++**：C++11标准，智能指针，lambda支持

## 📁 项目结构

```
threadpool/
├── include/                    # 头文件目录
│   ├── thread_pool.h          # 抽象接口定义
│   ├── fixed_thread_pool.h    # 固定大小线程池
│   ├── cached_thread_pool.h   # 缓存线程池
│   ├── priority_thread_pool.h # 优先级线程池
│   └── thread_pool_factory.h  # 工厂类
├── src/                       # 源文件目录
│   ├── thread_pool.cpp
│   ├── fixed_thread_pool.cpp
│   ├── priority_thread_pool.cpp
│   └── thread_pool_factory.cpp
├── examples/                  # 示例程序
│   └── demo_basic.cpp
├── CMakeLists.txt            # 构建配置
└── README.md                 # 项目文档
```

## 🛠️ 编译安装

### 依赖要求

- CMake 3.10+
- C++11兼容编译器 (GCC 4.8+, Clang 3.3+, MSVC 2015+)
- pthread库

### 编译步骤

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make -j$(nproc)

# 运行演示程序
./bin/demo_threadpool
```

## 🎯 快速开始

### 基本使用

```cpp
#include "thread_pool_factory.h"
using namespace ThreadPool;

int main() {
    // 创建固定大小线程池
    auto pool = createFixedThreadPool(4);
    pool->start();
    
    // 提交任务
    pool->submit([]() {
        std::cout << "Hello from thread pool!" << std::endl;
    });
    
    // 有返回值的任务
    auto future = pool->submitWithResult([]() -> int {
        return 42;
    });
    
    int result = future.get();
    std::cout << "Result: " << result << std::endl;
    
    // 关闭线程池
    pool->shutdown();
    return 0;
}
```

### 工厂模式创建

```cpp
// 创建推荐的线程池
auto pool = ThreadPoolFactory::createRecommended();

// 创建指定类型
auto fixedPool = ThreadPoolFactory::create(ThreadPoolType::Fixed, 8);
auto priorityPool = ThreadPoolFactory::create(ThreadPoolType::Priority, 4);

// 根据场景推荐
auto type = ThreadPoolFactory::recommendType(
    ThreadPoolFactory::UsageScenario::IOIntensive
);
auto ioPool = ThreadPoolFactory::create(type);
```

## 📊 线程池类型对比

| 类型 | 适用场景 | 特点 | 性能 |
|------|----------|------|------|
| **固定线程池** | CPU密集型 | 线程数固定，资源可控 | 高CPU利用率 |
| **缓存线程池** | IO密集型 | 动态创建回收线程 | 高并发处理 |
| **优先级线程池** | 任务有优先级 | 支持任务优先级调度 | 灵活调度 |
| **定时任务线程池** | 定时执行 | 支持延迟和定时执行 | 时间精确 |
| **工作窃取线程池** | 递归分治 | 负载均衡，减少等待 | 高吞吐量 |

## 🔧 配置选项

### 线程池配置

```cpp
ThreadPoolConfig config;
config.coreThreads = 4;              // 核心线程数
config.maxThreads = 8;               // 最大线程数
config.maxQueueSize = 1000;          // 队列容量
config.keepAliveTime = std::chrono::milliseconds(60000); // 空闲存活时间
config.allowCoreThreadTimeout = false; // 核心线程超时
config.threadNamePrefix = "Worker-";  // 线程名前缀

auto pool = ThreadPoolFactory::create(ThreadPoolType::Fixed, config);
```

### 拒绝策略

```cpp
auto pool = createFixedThreadPool(2);
auto fixedPool = dynamic_cast<FixedThreadPool*>(pool.get());

// 设置拒绝策略
fixedPool->setRejectionPolicy(RejectionPolicy::Discard);    // 丢弃任务
fixedPool->setRejectionPolicy(RejectionPolicy::CallerRuns); // 调用者执行
fixedPool->setRejectionPolicy(RejectionPolicy::Block);      // 阻塞等待
```

## 📈 监控统计

```cpp
// 获取统计信息
auto stats = pool->getStats();
std::cout << stats.toString() << std::endl;

// 输出示例：
// 线程池统计信息:
//   线程数量: 4
//   活跃线程: 2
//   队列长度: 15/1000
//   已完成任务: 1250
//   被拒绝任务: 3
//   平均执行时间: 125.50ms
```

## 🎨 高级功能

### 优先级任务

```cpp
auto pool = createPriorityThreadPool(4);
auto priorityPool = dynamic_cast<PriorityThreadPool*>(pool.get());

// 提交不同优先级的任务
priorityPool->submitWithPriority(lowPriorityTask, 1);   // 低优先级
priorityPool->submitWithPriority(highPriorityTask, 10); // 高优先级

// 批量提交
std::vector<std::pair<Task, int>> tasks = {
    {task1, 5}, {task2, 8}, {task3, 3}
};
priorityPool->submitBatchWithPriority(tasks);
```

### 批量操作

```cpp
// 批量提交任务
std::vector<Task> tasks;
for (int i = 0; i < 100; ++i) {
    tasks.push_back([i]() { processData(i); });
}

size_t submitted = pool->submitBatch(tasks);
std::cout << "成功提交 " << submitted << " 个任务" << std::endl;
```

### 性能基准测试

```cpp
// 比较不同线程池性能
std::vector<ThreadPoolType> types = {
    ThreadPoolType::Fixed,
    ThreadPoolType::Priority
};

auto results = ThreadPoolFactory::benchmark(types, 10000);
for (const auto& result : results) {
    std::cout << result.toString() << std::endl;
}
```

## 🔍 使用场景推荐

### CPU密集型任务
```cpp
// 推荐：固定线程池，线程数 = CPU核心数
auto pool = createFixedThreadPool(std::thread::hardware_concurrency());
```

### IO密集型任务
```cpp
// 推荐：缓存线程池，动态调整线程数
auto pool = createCachedThreadPool();
```

### 混合型任务
```cpp
// 推荐：使用工厂推荐算法
auto pool = ThreadPoolFactory::createRecommended();
```

### 有优先级需求
```cpp
// 推荐：优先级线程池
auto pool = createPriorityThreadPool(4);
```

## ⚠️ 注意事项

1. **资源管理**：确保在程序退出前调用`shutdown()`
2. **异常处理**：任务中的异常会被捕获并记录
3. **死锁避免**：避免在任务中等待同一线程池的其他任务
4. **内存使用**：大队列可能消耗较多内存
5. **线程安全**：所有公共接口都是线程安全的

## 🚧 待实现功能

- [ ] 缓存线程池完整实现
- [ ] 定时任务线程池
- [ ] 工作窃取线程池
- [ ] 更详细的性能监控
- [ ] 配置文件支持
- [ ] 日志集成

## 🤝 贡献指南

欢迎提交Issue和Pull Request！

## 📄 许可证

MIT License

---

**线程池框架** - 让并发编程更简单！ 