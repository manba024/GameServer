#pragma once

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <chrono>

namespace ThreadPool {

// 任务类型定义
using Task = std::function<void()>;

// 线程池统计信息
struct ThreadPoolStats {
    size_t threadCount = 0;         // 线程数量
    size_t activeThreads = 0;       // 活跃线程数
    size_t queueSize = 0;           // 队列中的任务数
    size_t maxQueueSize = 0;        // 最大队列容量
    size_t completedTasks = 0;      // 已完成任务数
    size_t rejectedTasks = 0;       // 被拒绝的任务数
    double avgExecutionTime = 0.0;  // 平均执行时间（毫秒）
    
    std::string toString() const;
};

// 线程池配置
struct ThreadPoolConfig {
    size_t coreThreads = 4;         // 核心线程数
    size_t maxThreads = 8;          // 最大线程数
    size_t maxQueueSize = 1000;     // 最大队列长度
    std::chrono::milliseconds keepAliveTime{60000}; // 空闲线程存活时间
    bool allowCoreThreadTimeout = false;  // 是否允许核心线程超时
    std::string threadNamePrefix = "ThreadPool-"; // 线程名前缀
    
    ThreadPoolConfig() = default;
    ThreadPoolConfig(size_t cores, size_t maxQueue = 1000) 
        : coreThreads(cores), maxThreads(cores), maxQueueSize(maxQueue) {}
};

// 拒绝策略枚举
enum class RejectionPolicy {
    Abort,          // 抛出异常
    Discard,        // 丢弃任务
    DiscardOldest,  // 丢弃最旧的任务
    CallerRuns,     // 调用者线程执行
    Block           // 阻塞等待
};

// 线程池抽象基类
class IThreadPool {
public:
    virtual ~IThreadPool() = default;

    // 提交任务（无返回值）
    virtual bool submit(Task task) = 0;
    
    // 提交任务（有返回值）
    template<typename F, typename... Args>
    auto submitWithResult(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    
    // 批量提交任务
    virtual size_t submitBatch(const std::vector<Task>& tasks) = 0;
    
    // 启动线程池
    virtual bool start() = 0;
    
    // 停止线程池
    virtual void stop() = 0;
    
    // 优雅关闭（等待所有任务完成）
    virtual void shutdown() = 0;
    
    // 强制关闭（立即停止）
    virtual void shutdownNow() = 0;
    
    // 等待所有任务完成
    virtual bool awaitTermination(std::chrono::milliseconds timeout) = 0;
    
    // 获取统计信息
    virtual ThreadPoolStats getStats() const = 0;
    
    // 获取配置信息
    virtual ThreadPoolConfig getConfig() const = 0;
    
    // 动态调整线程数
    virtual bool setCorePoolSize(size_t coreSize) = 0;
    virtual bool setMaximumPoolSize(size_t maxSize) = 0;
    
    // 状态查询
    virtual bool isRunning() const = 0;
    virtual bool isShutdown() const = 0;
    virtual bool isTerminated() const = 0;
    
    // 获取线程池类型名称
    virtual std::string getTypeName() const = 0;
};

// 线程池类型枚举
enum class ThreadPoolType {
    Fixed,          // 固定大小线程池
    Cached,         // 缓存线程池
    Scheduled,      // 定时任务线程池
    WorkStealing,   // 工作窃取线程池
    Priority        // 优先级线程池
};

// 实现有返回值的任务提交
template<typename F, typename... Args>
auto IThreadPool::submitWithResult(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type> {
    
    using ReturnType = typename std::result_of<F(Args...)>::type;
    
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<ReturnType> result = task->get_future();
    
    // 提交包装后的任务
    bool submitted = submit([task]() { (*task)(); });
    
    if (!submitted) {
        // 如果提交失败，返回一个异常future
        std::promise<ReturnType> promise;
        promise.set_exception(std::make_exception_ptr(
            std::runtime_error("Task submission failed")
        ));
        return promise.get_future();
    }
    
    return result;
}

} // namespace ThreadPool 