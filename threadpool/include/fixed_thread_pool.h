#pragma once

#include "thread_pool.h"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>

namespace ThreadPool {

// 固定大小线程池实现
class FixedThreadPool : public IThreadPool {
public:
    explicit FixedThreadPool(const ThreadPoolConfig& config);
    explicit FixedThreadPool(size_t threadCount);
    ~FixedThreadPool() override;

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
    
    std::vector<std::thread> workers_;
    std::queue<Task> taskQueue_;
    
    mutable std::mutex queueMutex_;
    std::condition_variable condition_;
    std::condition_variable terminationCondition_;
    
    std::atomic<bool> running_{false};
    std::atomic<bool> shutdown_{false};
    std::atomic<bool> terminated_{false};
    std::atomic<size_t> activeThreads_{0};
    std::atomic<size_t> completedTasks_{0};
    std::atomic<size_t> rejectedTasks_{0};
    
    // 工作线程函数
    void workerThread(size_t threadId);
    
    // 拒绝策略处理
    bool handleRejection(const Task& task);
    
    // 统计相关
    mutable std::mutex statsMutex_;
    std::vector<std::chrono::high_resolution_clock::time_point> taskStartTimes_;
    std::vector<double> executionTimes_;
    
    void recordTaskStart();
    void recordTaskEnd();
    double calculateAvgExecutionTime() const;
};

} // namespace ThreadPool 