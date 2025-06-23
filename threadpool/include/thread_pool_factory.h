#pragma once

#include "thread_pool.h"
#include "fixed_thread_pool.h"
#include "cached_thread_pool.h"
#include "priority_thread_pool.h"
#include <memory>
#include <string>
#include <vector>

namespace ThreadPool {

// 线程池工厂类
class ThreadPoolFactory {
public:
    // 创建指定类型的线程池
    static std::unique_ptr<IThreadPool> create(ThreadPoolType type, const ThreadPoolConfig& config);
    static std::unique_ptr<IThreadPool> create(ThreadPoolType type, size_t threadCount = 4);
    
    // 创建推荐的线程池（根据系统和使用场景）
    static std::unique_ptr<IThreadPool> createRecommended(const ThreadPoolConfig& config);
    static std::unique_ptr<IThreadPool> createRecommended(size_t threadCount = 0);
    
    // 获取支持的线程池类型
    static std::vector<ThreadPoolType> getSupportedTypes();
    
    // 获取类型名称
    static std::string getTypeName(ThreadPoolType type);
    
    // 获取类型描述
    static std::string getTypeDescription(ThreadPoolType type);
    
    // 根据场景推荐线程池类型
    enum class UsageScenario {
        GeneralPurpose,     // 通用场景
        IOIntensive,        // IO密集型
        CPUIntensive,       // CPU密集型
        HighThroughput,     // 高吞吐量
        LowLatency,         // 低延迟
        PriorityBased       // 基于优先级
    };
    
    static ThreadPoolType recommendType(UsageScenario scenario);
    static std::string getScenarioDescription(UsageScenario scenario);
    
    // 性能测试和比较
    struct PerformanceMetrics {
        std::string poolType;
        double avgThroughput;      // 平均吞吐量（任务/秒）
        double avgLatency;         // 平均延迟（毫秒）
        double cpuUsage;           // CPU使用率
        double memoryUsage;        // 内存使用量（MB）
        size_t completedTasks;     // 完成任务数
        
        std::string toString() const;
    };
    
    // 性能基准测试
    static std::vector<PerformanceMetrics> benchmark(
        const std::vector<ThreadPoolType>& types,
        size_t taskCount = 10000,
        std::chrono::milliseconds testDuration = std::chrono::milliseconds(5000)
    );
    
private:
    // 禁止实例化
    ThreadPoolFactory() = delete;
    ~ThreadPoolFactory() = delete;
    ThreadPoolFactory(const ThreadPoolFactory&) = delete;
    ThreadPoolFactory& operator=(const ThreadPoolFactory&) = delete;
    
    // 辅助函数
    static size_t getRecommendedThreadCount();
    static ThreadPoolConfig getDefaultConfig(ThreadPoolType type);
};

// 便捷函数
inline std::unique_ptr<IThreadPool> createThreadPool(ThreadPoolType type, size_t threadCount = 4) {
    return ThreadPoolFactory::create(type, threadCount);
}

inline std::unique_ptr<IThreadPool> createFixedThreadPool(size_t threadCount) {
    return ThreadPoolFactory::create(ThreadPoolType::Fixed, threadCount);
}

inline std::unique_ptr<IThreadPool> createCachedThreadPool() {
    return ThreadPoolFactory::create(ThreadPoolType::Cached);
}

inline std::unique_ptr<IThreadPool> createPriorityThreadPool(size_t threadCount) {
    return ThreadPoolFactory::create(ThreadPoolType::Priority, threadCount);
}

inline std::unique_ptr<IThreadPool> createRecommendedThreadPool(size_t threadCount = 0) {
    return ThreadPoolFactory::createRecommended(threadCount);
}

} // namespace ThreadPool 