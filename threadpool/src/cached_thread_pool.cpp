#include "../include/cached_thread_pool.h"
#include <algorithm>
#include <chrono>

namespace ThreadPool {

CachedThreadPool::CachedThreadPool(const ThreadPoolConfig& config) 
    : config_(config), rejectionPolicy_(RejectionPolicy::Abort) {
    if (config_.coreThreads > config_.maxThreads) {
        throw std::invalid_argument("Core pool size cannot be greater than maximum pool size");
    }
}

CachedThreadPool::CachedThreadPool() 
    : CachedThreadPool(ThreadPoolConfig{}) {
}

CachedThreadPool::~CachedThreadPool() {
    if (!terminated_) {
        shutdownNow();
    }
}

bool CachedThreadPool::submit(Task task) {
    if (shutdown_) {
        return handleRejection(task);
    }
    
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        if (taskQueue_.size() >= config_.maxQueueSize) {
            return handleRejection(task);
        }
        taskQueue_.push(std::move(task));
    }
    
    // 如果没有空闲线程且未达到最大线程数，创建新线程
    if (activeThreads_ >= workers_.size() && workers_.size() < config_.maxThreads) {
        createNewWorker();
    }
    
    condition_.notify_one();
    return true;
}

size_t CachedThreadPool::submitBatch(const std::vector<Task>& tasks) {
    size_t submitted = 0;
    for (const auto& task : tasks) {
        if (submit(task)) {
            submitted++;
        }
    }
    return submitted;
}

bool CachedThreadPool::start() {
    if (running_) {
        return false;
    }
    
    running_ = true;
    shutdown_ = false;
    terminated_ = false;
    
    // 创建核心线程
    for (size_t i = 0; i < config_.coreThreads; i++) {
        createNewWorker();
    }
    
    // 启动清理线程
    cleanupRunning_ = true;
    cleanupThread_ = std::thread(&CachedThreadPool::cleanupWorker, this);
    
    return true;
}

void CachedThreadPool::stop() {
    shutdown_ = true;
    condition_.notify_all();
}

void CachedThreadPool::shutdown() {
    stop();
}

void CachedThreadPool::shutdownNow() {
    shutdown_ = true;
    cleanupRunning_ = false;
    
    // 清空任务队列
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        std::queue<Task> empty;
        taskQueue_.swap(empty);
    }
    
    condition_.notify_all();
    
    // 等待所有线程结束
    joinAllWorkers();
    
    if (cleanupThread_.joinable()) {
        cleanupThread_.join();
    }
    
    running_ = false;
    terminated_ = true;
    terminationCondition_.notify_all();
}

bool CachedThreadPool::awaitTermination(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(queueMutex_);
    return terminationCondition_.wait_for(lock, timeout, [&] { return terminated_.load(); });
}

ThreadPoolStats CachedThreadPool::getStats() const {
    ThreadPoolStats stats;
    stats.threadCount = workers_.size();
    stats.maxQueueSize = config_.maxQueueSize;
    stats.activeThreads = activeThreads_;
    stats.queueSize = [this]() {
        std::lock_guard<std::mutex> lock(queueMutex_);
        return taskQueue_.size();
    }();
    stats.completedTasks = completedTasks_;
    stats.rejectedTasks = rejectedTasks_;
    stats.avgExecutionTime = calculateAvgExecutionTime();
    return stats;
}

ThreadPoolConfig CachedThreadPool::getConfig() const {
    return config_;
}

bool CachedThreadPool::setCorePoolSize(size_t coreSize) {
    if (coreSize > config_.maxThreads) {
        return false;
    }
    config_.coreThreads = coreSize;
    return true;
}

bool CachedThreadPool::setMaximumPoolSize(size_t maxSize) {
    if (maxSize < config_.coreThreads) {
        return false;
    }
    config_.maxThreads = maxSize;
    return true;
}

bool CachedThreadPool::isRunning() const {
    return running_;
}

bool CachedThreadPool::isShutdown() const {
    return shutdown_;
}

bool CachedThreadPool::isTerminated() const {
    return terminated_;
}

std::string CachedThreadPool::getTypeName() const {
    return "CachedThreadPool";
}

void CachedThreadPool::setRejectionPolicy(RejectionPolicy policy) {
    rejectionPolicy_ = policy;
}

void CachedThreadPool::workerThread(size_t workerId) {
    while (!shutdown_) {
        Task task;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this] { return shutdown_ || !taskQueue_.empty(); });
            
            if (taskQueue_.empty()) {
                if (shutdown_) break;
                continue;
            }
            
            task = std::move(taskQueue_.front());
            taskQueue_.pop();
        }
        
        // 执行任务
        activeThreads_++;
        
        // 更新工作线程活跃时间
        {
            std::lock_guard<std::mutex> lock(workersMutex_);
            if (workerId < workers_.size() && workers_[workerId]) {
                workers_[workerId]->lastActiveTime = std::chrono::steady_clock::now();
                workers_[workerId]->idle = false;
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            task();
        } catch (...) {
            // 忽略任务执行异常
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double executionTime = std::chrono::duration<double, std::milli>(end - start).count();
        recordTaskExecution(executionTime);
        
        completedTasks_++;
        activeThreads_--;
        
        // 标记为空闲
        {
            std::lock_guard<std::mutex> lock(workersMutex_);
            if (workerId < workers_.size() && workers_[workerId]) {
                workers_[workerId]->idle = true;
            }
        }
    }
}

void CachedThreadPool::cleanupIdleThreads() {
    std::lock_guard<std::mutex> lock(workersMutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto keepAliveTime = config_.keepAliveTime;
    
    for (auto it = workers_.begin(); it != workers_.end();) {
        auto& worker = *it;
        
        if (worker && worker->idle && 
            (now - worker->lastActiveTime) > keepAliveTime &&
            workers_.size() > config_.coreThreads) {
            
            if (worker->thread.joinable()) {
                worker->thread.join();
            }
            it = workers_.erase(it);
        } else {
            ++it;
        }
    }
}

bool CachedThreadPool::createNewWorker() {
    std::lock_guard<std::mutex> lock(workersMutex_);
    
    if (workers_.size() >= config_.maxThreads) {
        return false;
    }
    
    size_t workerId = nextWorkerId_++;
    std::unique_ptr<WorkerInfo> worker(new WorkerInfo());
    worker->lastActiveTime = std::chrono::steady_clock::now();
    worker->idle = true;
    
    try {
        worker->thread = std::thread(&CachedThreadPool::workerThread, this, workerId);
        workers_.push_back(std::move(worker));
        return true;
    } catch (...) {
        return false;
    }
}

void CachedThreadPool::joinAllWorkers() {
    std::lock_guard<std::mutex> lock(workersMutex_);
    
    for (auto& worker : workers_) {
        if (worker && worker->thread.joinable()) {
            worker->thread.join();
        }
    }
    workers_.clear();
}

bool CachedThreadPool::handleRejection(const Task& task) {
    rejectedTasks_++;
    
    switch (rejectionPolicy_) {
        case RejectionPolicy::Abort:
            return false;
        case RejectionPolicy::CallerRuns:
            if (!shutdown_) {
                try {
                    task();
                    return true;
                } catch (...) {
                    return false;
                }
            }
            return false;
        case RejectionPolicy::Discard:
            return false;
        case RejectionPolicy::DiscardOldest: {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (!taskQueue_.empty()) {
                taskQueue_.pop();
                taskQueue_.push(task);
                return true;
            }
            return false;
        }
        default:
            return false;
    }
}

void CachedThreadPool::recordTaskExecution(double executionTime) {
    std::lock_guard<std::mutex> lock(statsMutex_);
    executionTimes_.push_back(executionTime);
    
    // 保持最近1000次执行时间
    if (executionTimes_.size() > 1000) {
        executionTimes_.erase(executionTimes_.begin());
    }
}

double CachedThreadPool::calculateAvgExecutionTime() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    
    if (executionTimes_.empty()) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (double time : executionTimes_) {
        sum += time;
    }
    
    return sum / executionTimes_.size();
}

void CachedThreadPool::cleanupWorker() {
    while (cleanupRunning_) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        if (!shutdown_) {
            cleanupIdleThreads();
        }
    }
}

} // namespace ThreadPool 