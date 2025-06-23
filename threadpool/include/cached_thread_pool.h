#pragma once

#include "thread_pool.h"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace ThreadPool {

// 缓存线程池实现（根据需要动态创建和回收线程）
class CachedThreadPool : public IThreadPool {
public:
    explicit CachedThreadPool(const ThreadPoolConfig& config);
    CachedThreadPool();
    ~CachedThreadPool() override;

    // IThreadPool接口实现
    bool submit(Task task) override;
    size_t submitBatch(const std::vector<Task>& tasks) override;
    bool start() override;
    void stop() override;
    void shutdown() override;
    void shutdownNow() override;
    bool awaitTermination(std::chrono::milliseconds timeout) override;
    ThreadPoolStats getStats() const override;
    ThreadPoolConfig getConfig() const override;
    bool setCorePoolSize(size_t coreSize) override;
    bool setMaximumPoolSize(size_t maxSize) override;
    bool isRunning() const override;
    bool isShutdown() const override;
    bool isTerminated() const override;
    std::string getTypeName() const override;

    // 设置拒绝策略
    void setRejectionPolicy(RejectionPolicy policy);

private:
    ThreadPoolConfig config_;
    RejectionPolicy rejectionPolicy_;
    
    // 线程管理
    struct WorkerInfo {
        std::thread thread;
        std::chrono::steady_clock::time_point lastActiveTime;
        std::atomic<bool> idle{true};
        
        WorkerInfo() = default;
        WorkerInfo(WorkerInfo&& other) noexcept 
            : thread(std::move(other.thread)), 
              lastActiveTime(other.lastActiveTime),
              idle(other.idle.load()) {}
    };
    
    std::vector<std::unique_ptr<WorkerInfo>> workers_;
    std::queue<Task> taskQueue_;
    
    mutable std::mutex workersMutex_;
    mutable std::mutex queueMutex_;
    std::condition_variable condition_;
    std::condition_variable terminationCondition_;
    
    std::atomic<bool> running_{false};
    std::atomic<bool> shutdown_{false};
    std::atomic<bool> terminated_{false};
    std::atomic<size_t> activeThreads_{0};
    std::atomic<size_t> completedTasks_{0};
    std::atomic<size_t> rejectedTasks_{0};
    std::atomic<size_t> nextWorkerId_{0};
    
    // 线程管理
    void workerThread(size_t workerId);
    void cleanupIdleThreads();
    bool createNewWorker();
    void joinAllWorkers();
    
    // 拒绝策略处理
    bool handleRejection(const Task& task);
    
    // 统计相关
    mutable std::mutex statsMutex_;
    std::vector<double> executionTimes_;
    
    void recordTaskExecution(double executionTime);
    double calculateAvgExecutionTime() const;
    
    // 清理线程
    std::thread cleanupThread_;
    std::atomic<bool> cleanupRunning_{false};
    void cleanupWorker();
};

} // namespace ThreadPool 