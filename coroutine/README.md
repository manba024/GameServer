# 协程库 (Coroutine Library)

一个基于C++11的高性能协程库，提供了完整的协程调度、同步原语和通信机制。

## 🚀 特性

### 核心功能
- **协程调度器** - 支持协程的创建、调度和管理
- **协程同步** - 提供互斥锁、条件变量、信号量等同步原语
- **协程通信** - 支持基于通道的消息传递机制
- **异常处理** - 完善的协程异常处理机制
- **内存管理** - 自动的协程栈空间管理

### 同步原语
- **Mutex** - 协程互斥锁
- **ConditionVariable** - 协程条件变量
- **Semaphore** - 协程信号量
- **Channel** - 协程通道（支持有缓冲和无缓冲）
- **LockGuard** - RAII风格的锁管理

### 高级特性
- **协程池** - 支持协程池模式
- **任务调度** - 灵活的任务调度机制
- **管道处理** - 支持协程管道模式
- **异步操作** - 支持异步HTTP请求等操作

## 📦 安装和编译

### 系统要求
- C++11 或更高版本
- CMake 3.10 或更高版本
- GCC 4.8+ 或 Clang 3.4+

### 编译步骤

```bash
# 在项目根目录下
mkdir build && cd build
cmake ..
make

# 编译协程库
make coroutine

# 编译示例程序
make coroutine_basic_example
make coroutine_advanced_example
```

### 运行示例

```bash
# 运行基本示例
./bin/coroutine_basic_example

# 运行高级示例
./bin/coroutine_advanced_example
```

## 📚 使用指南

### 基本用法

```cpp
#include "coroutine.h"
#include <iostream>

using namespace coroutine;

void my_coroutine(int id) {
    std::cout << "协程 " << id << " 开始执行" << std::endl;
    
    for (int i = 0; i < 3; ++i) {
        std::cout << "协程 " << id << " 步骤 " << i << std::endl;
        api::yield(); // 让出CPU
    }
    
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

### 互斥锁使用

```cpp
#include "sync.h"

coroutine::Mutex mutex;
int shared_counter = 0;

void increment_counter(int id) {
    for (int i = 0; i < 10; ++i) {
        coroutine::LockGuard lock(mutex);
        shared_counter++;
        std::cout << "协程 " << id << " 增加计数器到 " << shared_counter << std::endl;
        coroutine::api::yield();
    }
}
```

### 通道通信

```cpp
#include "sync.h"

coroutine::Channel<int> channel(5); // 容量为5的通道

void producer() {
    for (int i = 1; i <= 10; ++i) {
        channel.send(i);
        std::cout << "生产数据: " << i << std::endl;
        coroutine::api::yield();
    }
    channel.close();
}

void consumer() {
    int data;
    while (channel.receive(data)) {
        std::cout << "消费数据: " << data << std::endl;
        coroutine::api::yield();
    }
}
```

### 信号量使用

```cpp
#include "sync.h"

coroutine::Semaphore semaphore(2); // 最多2个协程同时访问

void access_resource(int id) {
    semaphore.acquire();
    std::cout << "协程 " << id << " 获得资源" << std::endl;
    
    // 使用资源
    coroutine::api::yield();
    
    std::cout << "协程 " << id << " 释放资源" << std::endl;
    semaphore.release();
}
```

## 🏗️ 架构设计

### 核心组件

```
┌─────────────────────────────────────────────────────────────┐
│                      协程库架构图                            │
├─────────────────────────────────────────────────────────────┤
│  API Layer                                                  │
│  ┌─────────────────────────────────────────────────────────┐│
│  │ create() │ yield() │ resume() │ run() │ stop()          ││
│  └─────────────────────────────────────────────────────────┘│
├─────────────────────────────────────────────────────────────┤
│  Scheduler Layer                                            │
│  ┌─────────────────────────────────────────────────────────┐│
│  │ Scheduler │ Ready Queue │ Context Switch │ Memory Mgmt ││
│  └─────────────────────────────────────────────────────────┘│
├─────────────────────────────────────────────────────────────┤
│  Coroutine Layer                                            │
│  ┌─────────────────────────────────────────────────────────┐│
│  │ Coroutine │ Stack Space │ Context │ State Machine      ││
│  └─────────────────────────────────────────────────────────┘│
├─────────────────────────────────────────────────────────────┤
│  Sync Layer                                                 │
│  ┌─────────────────────────────────────────────────────────┐│
│  │ Mutex │ CondVar │ Semaphore │ Channel │ LockGuard       ││
│  └─────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────┘
```

### 协程状态机

```
    ┌─────────┐
    │  READY  │
    └─────────┘
         │
         ▼
    ┌─────────┐    yield()    ┌─────────────┐
    │ RUNNING │◄──────────────┤ SUSPENDED   │
    └─────────┘               └─────────────┘
         │                           ▲
         │                           │
         │ finish()            resume()
         ▼                           │
    ┌─────────┐                     │
    │FINISHED │─────────────────────┘
    └─────────┘
```

## 🔧 API参考

### 核心API

#### `coroutine::api::create(func, stack_size)`
创建一个新的协程。

**参数:**
- `func`: 协程执行函数
- `stack_size`: 栈大小（可选，默认128KB）

**返回值:** 协程ID

#### `coroutine::api::yield()`
挂起当前协程，让出CPU给其他协程。

#### `coroutine::api::resume(id)`
恢复指定协程的执行。

**参数:**
- `id`: 协程ID

#### `coroutine::api::run()`
启动协程调度器，开始执行协程。

#### `coroutine::api::stop()`
停止协程调度器。

#### `coroutine::api::current()`
获取当前执行的协程ID。

**返回值:** 当前协程ID

### 同步原语API

#### `coroutine::Mutex`
协程互斥锁。

```cpp
void lock();        // 加锁
bool try_lock();    // 尝试加锁
void unlock();      // 解锁
bool is_locked();   // 检查是否已锁定
```

#### `coroutine::ConditionVariable`
协程条件变量。

```cpp
void wait(Mutex& mutex);                    // 等待条件
bool wait_for(Mutex& mutex, int timeout);  // 超时等待
void notify_one();                          // 唤醒一个等待者
void notify_all();                          // 唤醒所有等待者
```

#### `coroutine::Semaphore`
协程信号量。

```cpp
void acquire();      // 获取信号量
bool try_acquire();  // 尝试获取信号量
void release();      // 释放信号量
int count();         // 获取当前计数
```

#### `coroutine::Channel<T>`
协程通道。

```cpp
bool send(const T& data);        // 发送数据
bool receive(T& data);           // 接收数据
bool try_send(const T& data);    // 尝试发送数据
bool try_receive(T& data);       // 尝试接收数据
void close();                    // 关闭通道
bool is_closed();                // 检查是否已关闭
size_t size();                   // 获取缓冲区大小
bool empty();                    // 检查是否为空
bool full();                     // 检查是否已满
```

## 📊 性能特性

### 内存使用
- **默认栈大小**: 128KB每个协程
- **最小栈大小**: 4KB
- **内存对齐**: 16字节对齐
- **内存池**: 支持栈内存复用

### 调度性能
- **上下文切换**: 基于setjmp/longjmp实现
- **调度算法**: 轮转调度
- **时间复杂度**: O(1)调度开销
- **空间复杂度**: O(n)内存使用

### 并发能力
- **协程数量**: 理论上无限制（受内存限制）
- **同步开销**: 最小化锁竞争
- **通道性能**: 高效的生产者-消费者模式

## 🧪 测试和示例

### 基本示例
运行基本示例来了解协程的基本用法：

```bash
./bin/coroutine_basic_example
```

包含的示例：
- 基本协程调度
- 生产者-消费者模式
- 互斥锁使用
- 信号量使用

### 高级示例
运行高级示例来了解复杂的协程应用：

```bash
./bin/coroutine_advanced_example
```

包含的示例：
- 协程池
- 异步HTTP客户端
- 任务调度器
- 协程管道
- 协程间通信

## 🚨 注意事项

### 使用限制
1. **栈大小**: 确保栈大小足够大，避免栈溢出
2. **异常处理**: 协程函数中的异常会被捕获并打印
3. **资源管理**: 确保在协程结束前释放所有资源
4. **线程安全**: 当前实现是单线程的，不支持多线程环境

### 最佳实践
1. **合理让出**: 在适当的时候调用`yield()`
2. **避免死锁**: 注意锁的获取顺序
3. **通道关闭**: 及时关闭不再使用的通道
4. **内存管理**: 避免在协程栈上分配大量内存

## 🔮 未来计划

### 短期目标
- [ ] 支持多线程环境
- [ ] 添加协程优先级调度
- [ ] 实现更高效的上下文切换
- [ ] 添加协程调试工具

### 长期目标
- [ ] 支持协程网络IO
- [ ] 实现协程定时器
- [ ] 添加协程性能分析工具
- [ ] 支持协程序列化

## 🤝 贡献指南

欢迎提交Issue和Pull Request来改进这个协程库！

### 开发环境
1. Fork本项目
2. 创建特性分支
3. 编写测试用例
4. 提交Pull Request

### 编码规范
- 使用C++11标准
- 遵循现有的代码风格
- 添加必要的注释
- 编写单元测试

## 📄 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 📞 联系方式

如有问题或建议，请通过以下方式联系：
- 提交Issue
- 发送邮件
- 参与讨论

---

**GameServer协程库** - 让异步编程更简单！ 