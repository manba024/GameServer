#include "../include/priority_thread_pool.h"
#include <iostream>
#include <chrono>

namespace ThreadPool {

PriorityThreadPool::PriorityThreadPool(const ThreadPoolConfig& config) 
    : config_(config), rejectionPolicy_(RejectionPolicy::Abort), defaultPriority_(0) {
    
    if (config_.coreThreads == 0) {
        config_.coreThreads = 1;
    }
    config_.maxThreads = config_.coreThreads; // 固定线程数
    
    std::cout << "创建优先级线程池，线程数: " << config_.coreThreads << std::endl;
}

PriorityThreadPool::PriorityThreadPool(size_t threadCount) 
    : PriorityThreadPool(ThreadPoolConfig(threadCount)) {
}

PriorityThreadPool::~PriorityThreadPool() {
    if (running_) {
        shutdownNow();
    }
}

bool PriorityThreadPool::submit(Task task) {
    return submitWithPriority(std::move(task), defaultPriority_);
}

bool PriorityThreadPool::submitWithPriority(Task task, int priority) {
    if (shutdown_.load() || !running_.load()) {
        return handleRejection(task, priority);
    }
    
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        
        if (taskQueue_.size() >= config_.maxQueueSize) {
            return handleRejection(task, priority);
        }
        
        taskQueue_.emplace(std::move(task), priority);
    }
    
    condition_.notify_one();
    return true;
}

size_t PriorityThreadPool::submitBatch(const std::vector<Task>& tasks) {
    size_t submitted = 0;
    for (const auto& task : tasks) {
        if (submit(task)) {
            ++submitted;
        }
    }
    return submitted;
}

bool PriorityThreadPool::submitBatchWithPriority(const std::vector<std::pair<Task, int>>& tasks) {
    size_t submitted = 0;
    for (const auto& taskPair : tasks) {
        if (submitWithPriority(taskPair.first, taskPair.second)) {
            ++submitted;
        }
    }
    return submitted == tasks.size();
}

bool PriorityThreadPool::start() {
    if (running_.load()) {
        return false;
    }
    
    running_ = true;
    shutdown_ = false;
    terminated_ = false;
    
    workers_.reserve(config_.coreThreads);
    for (size_t i = 0; i < config_.coreThreads; ++i) {
        workers_.emplace_back(&PriorityThreadPool::workerThread, this, i);
    }
    
    std::cout << "优先级线程池已启动，线程数: " << config_.coreThreads << std::endl;
    return true;
}

void PriorityThreadPool::stop() {
    shutdown();
}

void PriorityThreadPool::shutdown() {
    if (!running_.load() || shutdown_.load()) {
        return;
    }
    
    std::cout << "正在关闭优先级线程池..." << std::endl;
    
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        shutdown_ = true;
    }
    
    condition_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
    running_ = false;
    terminated_ = true;
    
    terminationCondition_.notify_all();
    std::cout << "优先级线程池已关闭" << std::endl;
}

void PriorityThreadPool::shutdownNow() {
    if (!running_.load()) {
        return;
    }
    
    std::cout << "正在强制关闭优先级线程池..." << std::endl;
    
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        shutdown_ = true;
        
        // 清空队列
        std::priority_queue<PriorityTask> empty;
        taskQueue_.swap(empty);
    }
    
    condition_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
    running_ = false;
    terminated_ = true;
    
    terminationCondition_.notify_all();
    std::cout << "优先级线程池已强制关闭" << std::endl;
}

bool PriorityThreadPool::awaitTermination(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(queueMutex_);
    return terminationCondition_.wait_for(lock, timeout, [this] { return terminated_.load(); });
}

ThreadPoolStats PriorityThreadPool::getStats() const {
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

ThreadPoolConfig PriorityThreadPool::getConfig() const {
    return config_;
}

bool PriorityThreadPool::setCorePoolSize(size_t coreSize) {
    (void)coreSize; // 消除未使用参数警告
    return false; // 不支持动态调整
}

bool PriorityThreadPool::setMaximumPoolSize(size_t maxSize) {
    (void)maxSize; // 消除未使用参数警告
    return false; // 不支持动态调整
}

bool PriorityThreadPool::isRunning() const {
    return running_.load();
}

bool PriorityThreadPool::isShutdown() const {
    return shutdown_.load();
}

bool PriorityThreadPool::isTerminated() const {
    return terminated_.load();
}

std::string PriorityThreadPool::getTypeName() const {
    return "PriorityThreadPool";
}

void PriorityThreadPool::setDefaultPriority(int priority) {
    defaultPriority_ = priority;
}

int PriorityThreadPool::getDefaultPriority() const {
    return defaultPriority_;
}

void PriorityThreadPool::setRejectionPolicy(RejectionPolicy policy) {
    rejectionPolicy_ = policy;
}

void PriorityThreadPool::workerThread(size_t threadId) {
    std::cout << "优先级工作线程 " << threadId << " 启动" << std::endl;
    
    while (true) {
        PriorityTask priorityTask([](){}, 0);
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this] { return shutdown_.load() || !taskQueue_.empty(); });
            
            if (shutdown_.load() && taskQueue_.empty()) {
                break;
            }
            
            if (!taskQueue_.empty()) {
                priorityTask = std::move(const_cast<PriorityTask&>(taskQueue_.top()));
                taskQueue_.pop();
            }
        }
        
        if (priorityTask.task) {
            ++activeThreads_;
            
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                priorityTask.task();
            } catch (const std::exception& e) {
                std::cerr << "优先级线程 " << threadId << " 任务执行异常: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "优先级线程 " << threadId << " 任务执行未知异常" << std::endl;
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            recordTaskExecution(duration.count() / 1000.0, priorityTask.priority);
            
            ++completedTasks_;
            --activeThreads_;
        }
    }
    
    std::cout << "优先级工作线程 " << threadId << " 退出" << std::endl;
}

bool PriorityThreadPool::handleRejection(const Task& task, int priority) {
    (void)priority; // 消除未使用参数警告
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
                    taskQueue_.pop(); // 丢弃最高优先级的任务
                    taskQueue_.emplace(task, priority);
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
                
                taskQueue_.emplace(task, priority);
                lock.unlock();
                condition_.notify_one();
                return true;
            }
    }
    
    return false;
}

void PriorityThreadPool::recordTaskExecution(double executionTime, int priority) {
    std::unique_lock<std::mutex> lock(statsMutex_);
    
    executionTimes_.push_back(executionTime);
    if (executionTimes_.size() > 1000) {
        executionTimes_.erase(executionTimes_.begin());
    }
    
    priorityStats_[priority]++;
}

double PriorityThreadPool::calculateAvgExecutionTime() const {
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