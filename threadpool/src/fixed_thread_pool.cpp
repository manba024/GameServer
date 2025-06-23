#include "../include/fixed_thread_pool.h"
#include <iostream>
#include <chrono>
#include <algorithm>

namespace ThreadPool {

FixedThreadPool::FixedThreadPool(const ThreadPoolConfig& config) 
    : config_(config), rejectionPolicy_(RejectionPolicy::Abort) {
    
    // 确保配置合理
    if (config_.coreThreads == 0) {
        config_.coreThreads = 1;
    }
    config_.maxThreads = config_.coreThreads; // 固定线程池大小相等
    
    std::cout << "创建固定线程池，线程数: " << config_.coreThreads << std::endl;
}

FixedThreadPool::FixedThreadPool(size_t threadCount) 
    : FixedThreadPool(ThreadPoolConfig(threadCount)) {
}

FixedThreadPool::~FixedThreadPool() {
    if (running_) {
        shutdownNow();
    }
}

bool FixedThreadPool::submit(Task task) {
    if (shutdown_ || !running_) {
        return handleRejection(task);
    }
    
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        
        if (taskQueue_.size() >= config_.maxQueueSize) {
            return handleRejection(task);
        }
        
        taskQueue_.push(std::move(task));
    }
    
    condition_.notify_one();
    return true;
}

size_t FixedThreadPool::submitBatch(const std::vector<Task>& tasks) {
    size_t submitted = 0;
    
    for (const auto& task : tasks) {
        if (submit(task)) {
            ++submitted;
        }
    }
    
    return submitted;
}

bool FixedThreadPool::start() {
    if (running_) {
        return false;
    }
    
    running_ = true;
    shutdown_ = false;
    terminated_ = false;
    
    // 创建工作线程
    workers_.reserve(config_.coreThreads);
    for (size_t i = 0; i < config_.coreThreads; ++i) {
        workers_.emplace_back(&FixedThreadPool::workerThread, this, i);
    }
    
    std::cout << "固定线程池已启动，线程数: " << config_.coreThreads << std::endl;
    return true;
}

void FixedThreadPool::stop() {
    shutdown();
}

void FixedThreadPool::shutdown() {
    if (!running_ || shutdown_) {
        return;
    }
    
    std::cout << "正在关闭固定线程池..." << std::endl;
    
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        shutdown_ = true;
    }
    
    condition_.notify_all();
    
    // 等待所有线程完成
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
    running_ = false;
    terminated_ = true;
    
    terminationCondition_.notify_all();
    std::cout << "固定线程池已关闭" << std::endl;
}

void FixedThreadPool::shutdownNow() {
    if (!running_) {
        return;
    }
    
    std::cout << "正在强制关闭固定线程池..." << std::endl;
    
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        shutdown_ = true;
        
        // 清空队列中的任务
        std::queue<Task> empty;
        taskQueue_.swap(empty);
    }
    
    condition_.notify_all();
    
    // 等待所有线程完成
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
    running_ = false;
    terminated_ = true;
    
    terminationCondition_.notify_all();
    std::cout << "固定线程池已强制关闭" << std::endl;
}

bool FixedThreadPool::awaitTermination(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(queueMutex_);
    return terminationCondition_.wait_for(lock, timeout, [this] { return terminated_.load(); });
}

ThreadPoolStats FixedThreadPool::getStats() const {
    std::unique_lock<std::mutex> lock(queueMutex_);
    
    ThreadPoolStats stats;
    stats.threadCount = config_.coreThreads;
    stats.activeThreads = activeThreads_.load();
    stats.queueSize = taskQueue_.size();
    stats.maxQueueSize = config_.maxQueueSize;
    stats.completedTasks = completedTasks_.load();
    stats.rejectedTasks = rejectedTasks_.load();
    stats.avgExecutionTime = calculateAvgExecutionTime();
    
    return stats;
}

ThreadPoolConfig FixedThreadPool::getConfig() const {
    return config_;
}

bool FixedThreadPool::setCorePoolSize(size_t coreSize) {
    // 固定线程池不支持动态调整
    return false;
}

bool FixedThreadPool::setMaximumPoolSize(size_t maxSize) {
    // 固定线程池不支持动态调整
    return false;
}

bool FixedThreadPool::isRunning() const {
    return running_;
}

bool FixedThreadPool::isShutdown() const {
    return shutdown_;
}

bool FixedThreadPool::isTerminated() const {
    return terminated_;
}

std::string FixedThreadPool::getTypeName() const {
    return "FixedThreadPool";
}

void FixedThreadPool::setRejectionPolicy(RejectionPolicy policy) {
    rejectionPolicy_ = policy;
}

void FixedThreadPool::workerThread(size_t threadId) {
    std::cout << "工作线程 " << threadId << " 启动" << std::endl;
    
    while (true) {
        Task task;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this] { return shutdown_.load() || !taskQueue_.empty(); });
            
            if (shutdown_.load() && taskQueue_.empty()) {
                break;
            }
            
            if (!taskQueue_.empty()) {
                task = std::move(taskQueue_.front());
                taskQueue_.pop();
            }
        }
        
        if (task) {
            ++activeThreads_;
            recordTaskStart();
            
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "线程 " << threadId << " 任务执行异常: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "线程 " << threadId << " 任务执行未知异常" << std::endl;
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            recordTaskEnd();
            
            {
                std::unique_lock<std::mutex> lock(statsMutex_);
                executionTimes_.push_back(duration.count() / 1000.0); // 转换为毫秒
                if (executionTimes_.size() > 1000) { // 限制统计数据大小
                    executionTimes_.erase(executionTimes_.begin());
                }
            }
            
            ++completedTasks_;
            --activeThreads_;
        }
    }
    
    std::cout << "工作线程 " << threadId << " 退出" << std::endl;
}

bool FixedThreadPool::handleRejection(const Task& task) {
    ++rejectedTasks_;
    
    switch (rejectionPolicy_) {
        case RejectionPolicy::Abort:
            throw std::runtime_error("任务被拒绝：队列已满");
            
        case RejectionPolicy::Discard:
            std::cerr << "任务被丢弃：队列已满" << std::endl;
            return false;
            
        case RejectionPolicy::DiscardOldest:
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                if (!taskQueue_.empty()) {
                    taskQueue_.pop(); // 丢弃最旧的任务
                    taskQueue_.push(task);
                    lock.unlock();
                    condition_.notify_one();
                    return true;
                }
            }
            return false;
            
        case RejectionPolicy::CallerRuns:
            try {
                task(); // 在调用者线程中执行
                return true;
            } catch (...) {
                return false;
            }
            
        case RejectionPolicy::Block:
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                condition_.wait(lock, [this] { 
                    return shutdown_.load() || taskQueue_.size() < config_.maxQueueSize; 
                });
                
                if (shutdown_.load()) {
                    return false;
                }
                
                taskQueue_.push(task);
                lock.unlock();
                condition_.notify_one();
                return true;
            }
    }
    
    return false;
}

void FixedThreadPool::recordTaskStart() {
    // 可以在这里记录任务开始时间
}

void FixedThreadPool::recordTaskEnd() {
    // 可以在这里记录任务结束时间
}

double FixedThreadPool::calculateAvgExecutionTime() const {
    std::unique_lock<std::mutex> lock(statsMutex_);
    
    if (executionTimes_.empty()) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (double time : executionTimes_) {
        sum += time;
    }
    
    return sum / executionTimes_.size();
}

} // namespace ThreadPool 