#include "../include/thread_pool_factory.h"
#include <thread>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace ThreadPool {

std::unique_ptr<IThreadPool> ThreadPoolFactory::create(ThreadPoolType type, const ThreadPoolConfig& config) {
    switch (type) {
        case ThreadPoolType::Fixed:
            return std::unique_ptr<IThreadPool>(new FixedThreadPool(config));
            
        case ThreadPoolType::Cached:
            return std::unique_ptr<IThreadPool>(new CachedThreadPool(config));
            
        case ThreadPoolType::Priority:
            return std::unique_ptr<IThreadPool>(new PriorityThreadPool(config));
            
        case ThreadPoolType::Scheduled:
            // TODO: 实现定时任务线程池
            std::cerr << "定时任务线程池尚未实现，使用固定线程池替代" << std::endl;
            return std::unique_ptr<IThreadPool>(new FixedThreadPool(config));
            
        case ThreadPoolType::WorkStealing:
            // TODO: 实现工作窃取线程池
            std::cerr << "工作窃取线程池尚未实现，使用固定线程池替代" << std::endl;
            return std::unique_ptr<IThreadPool>(new FixedThreadPool(config));
            
        default:
            throw std::invalid_argument("不支持的线程池类型");
    }
}

std::unique_ptr<IThreadPool> ThreadPoolFactory::create(ThreadPoolType type, size_t threadCount) {
    ThreadPoolConfig config(threadCount);
    return create(type, config);
}

std::unique_ptr<IThreadPool> ThreadPoolFactory::createRecommended(const ThreadPoolConfig& config) {
    // 根据硬件特性推荐线程池类型
    size_t hardwareConcurrency = std::thread::hardware_concurrency();
    
    if (config.coreThreads <= hardwareConcurrency && config.maxQueueSize <= 100) {
        // 小规模任务，使用固定线程池
        return create(ThreadPoolType::Fixed, config);
    } else if (config.maxQueueSize > 1000) {
        // 大量任务，使用缓存线程池
        return create(ThreadPoolType::Cached, config);
    } else {
        // 默认使用固定线程池
        return create(ThreadPoolType::Fixed, config);
    }
}

std::unique_ptr<IThreadPool> ThreadPoolFactory::createRecommended(size_t threadCount) {
    if (threadCount == 0) {
        threadCount = getRecommendedThreadCount();
    }
    
    ThreadPoolConfig config(threadCount);
    return createRecommended(config);
}

std::vector<ThreadPoolType> ThreadPoolFactory::getSupportedTypes() {
    return {
        ThreadPoolType::Fixed,
        ThreadPoolType::Cached,
        ThreadPoolType::Priority
        // ThreadPoolType::Scheduled,    // 待实现
        // ThreadPoolType::WorkStealing  // 待实现
    };
}

std::string ThreadPoolFactory::getTypeName(ThreadPoolType type) {
    switch (type) {
        case ThreadPoolType::Fixed:      return "固定线程池";
        case ThreadPoolType::Cached:     return "缓存线程池";
        case ThreadPoolType::Scheduled:  return "定时任务线程池";
        case ThreadPoolType::WorkStealing: return "工作窃取线程池";
        case ThreadPoolType::Priority:   return "优先级线程池";
        default: return "未知类型";
    }
}

std::string ThreadPoolFactory::getTypeDescription(ThreadPoolType type) {
    switch (type) {
        case ThreadPoolType::Fixed:
            return "固定数量的工作线程，适合CPU密集型任务";
            
        case ThreadPoolType::Cached:
            return "根据需要动态创建和回收线程，适合IO密集型任务";
            
        case ThreadPoolType::Scheduled:
            return "支持定时和延迟执行的线程池";
            
        case ThreadPoolType::WorkStealing:
            return "支持工作窃取算法的线程池，适合递归分治任务";
            
        case ThreadPoolType::Priority:
            return "支持任务优先级调度的线程池";
            
        default:
            return "未知线程池类型";
    }
}

ThreadPoolType ThreadPoolFactory::recommendType(UsageScenario scenario) {
    switch (scenario) {
        case UsageScenario::GeneralPurpose:
            return ThreadPoolType::Fixed;
            
        case UsageScenario::IOIntensive:
            return ThreadPoolType::Cached;
            
        case UsageScenario::CPUIntensive:
            return ThreadPoolType::Fixed;
            
        case UsageScenario::HighThroughput:
            return ThreadPoolType::Cached;
            
        case UsageScenario::LowLatency:
            return ThreadPoolType::Fixed;
            
        case UsageScenario::PriorityBased:
            return ThreadPoolType::Priority;
            
        default:
            return ThreadPoolType::Fixed;
    }
}

std::string ThreadPoolFactory::getScenarioDescription(UsageScenario scenario) {
    switch (scenario) {
        case UsageScenario::GeneralPurpose:
            return "通用场景：平衡的CPU和IO操作";
            
        case UsageScenario::IOIntensive:
            return "IO密集型：大量文件、网络操作";
            
        case UsageScenario::CPUIntensive:
            return "CPU密集型：大量计算操作";
            
        case UsageScenario::HighThroughput:
            return "高吞吐量：需要处理大量任务";
            
        case UsageScenario::LowLatency:
            return "低延迟：要求快速响应";
            
        case UsageScenario::PriorityBased:
            return "优先级调度：任务有不同的重要性";
            
        default:
            return "未知场景";
    }
}

std::vector<ThreadPoolFactory::PerformanceMetrics> ThreadPoolFactory::benchmark(
    const std::vector<ThreadPoolType>& types,
    size_t taskCount,
    std::chrono::milliseconds testDuration) {
    
    std::vector<PerformanceMetrics> results;
    
    for (ThreadPoolType type : types) {
        try {
            std::cout << "正在测试 " << getTypeName(type) << "..." << std::endl;
            
            auto pool = create(type, getRecommendedThreadCount());
            pool->start();
            
            PerformanceMetrics metrics;
            metrics.poolType = getTypeName(type);
            
            auto start = std::chrono::high_resolution_clock::now();
            
            // 提交测试任务
            std::atomic<size_t> completedCount{0};
            for (size_t i = 0; i < taskCount; ++i) {
                pool->submit([&completedCount]() {
                    // 模拟一些工作
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    ++completedCount;
                });
            }
            
            // 等待测试完成或超时
            std::this_thread::sleep_for(testDuration);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            // 收集统计信息
            auto stats = pool->getStats();
            metrics.completedTasks = completedCount.load();
            metrics.avgThroughput = static_cast<double>(metrics.completedTasks) / 
                                   (duration.count() / 1000.0);
            metrics.avgLatency = stats.avgExecutionTime;
            metrics.cpuUsage = 0.0; // 简化实现
            metrics.memoryUsage = 0.0; // 简化实现
            
            pool->shutdown();
            results.push_back(metrics);
            
        } catch (const std::exception& e) {
            std::cerr << "测试 " << getTypeName(type) << " 时发生异常: " << e.what() << std::endl;
        }
    }
    
    return results;
}

size_t ThreadPoolFactory::getRecommendedThreadCount() {
    size_t hardwareConcurrency = std::thread::hardware_concurrency();
    
    if (hardwareConcurrency == 0) {
        return 4; // 默认值
    }
    
    // 通常推荐使用CPU核心数
    return hardwareConcurrency;
}

ThreadPoolConfig ThreadPoolFactory::getDefaultConfig(ThreadPoolType type) {
    ThreadPoolConfig config;
    config.coreThreads = getRecommendedThreadCount();
    
    switch (type) {
        case ThreadPoolType::Fixed:
            config.maxThreads = config.coreThreads;
            config.maxQueueSize = 1000;
            break;
            
        case ThreadPoolType::Cached:
            config.maxThreads = config.coreThreads * 2;
            config.maxQueueSize = 10000;
            config.keepAliveTime = std::chrono::milliseconds(60000);
            break;
            
        case ThreadPoolType::Priority:
            config.maxThreads = config.coreThreads;
            config.maxQueueSize = 1000;
            break;
            
        default:
            break;
    }
    
    return config;
}

std::string ThreadPoolFactory::PerformanceMetrics::toString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << poolType << " 性能指标:\n";
    oss << "  吞吐量: " << avgThroughput << " 任务/秒\n";
    oss << "  平均延迟: " << avgLatency << " 毫秒\n";
    oss << "  完成任务数: " << completedTasks << "\n";
    oss << "  CPU使用率: " << cpuUsage << "%\n";
    oss << "  内存使用: " << memoryUsage << " MB";
    return oss.str();
}

} // namespace ThreadPool 