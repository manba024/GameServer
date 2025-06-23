#pragma once

#include "thread_pool.h"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>

namespace ThreadPool {

// 优先级任务结构
struct PriorityTask {
    Task task;
    int priority;
    std::chrono::steady_clock::time_point submitTime;
    
    PriorityTask(Task t, int p) 
        : task(std::move(t)), priority(p), submitTime(std::chrono::steady_clock::now()) {}
    
    // 优先级比较（高优先级优先，相同优先级按提交时间）
    bool operator<(const PriorityTask& other) const {
        if (priority != other.priority) {
            return priority < other.priority; // 数字越大优先级越高
        }
        return submitTime > other.submitTime; // 提交时间越早优先级越高
    }
};

// 优先级线程池实现
class PriorityThreadPool : public IThreadPool {
public:
    explicit PriorityThreadPool(const ThreadPoolConfig& config);
    explicit PriorityThreadPool(size_t threadCount);
    ~PriorityThreadPool() override;

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

    // 优先级相关接口
    bool submitWithPriority(Task task, int priority);
    bool submitBatchWithPriority(const std::vector<std::pair<Task, int>>& tasks);
    
    // 设置默认优先级
    void setDefaultPriority(int priority);
    int getDefaultPriority() const;
    
    // 设置拒绝策略
    void setRejectionPolicy(RejectionPolicy policy);

private:
    ThreadPoolConfig config_;
    RejectionPolicy rejectionPolicy_;
    int defaultPriority_;
    
    std::vector<std::thread> workers_;
    std::priority_queue<PriorityTask> taskQueue_;
    
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
    bool handleRejection(const Task& task, int priority = 0);
    
    // 统计相关
    mutable std::mutex statsMutex_;
    std::vector<double> executionTimes_;
    std::unordered_map<int, size_t> priorityStats_; // 各优先级任务统计
    
    void recordTaskExecution(double executionTime, int priority);
    double calculateAvgExecutionTime() const;
};

} // namespace ThreadPool 