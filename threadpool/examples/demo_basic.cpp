#include "../include/thread_pool_factory.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <atomic>

using namespace ThreadPool;

// 模拟CPU密集型任务
void cpuIntensiveTask(int taskId, int workload) {
    std::cout << "CPU任务 " << taskId << " 开始执行，工作量: " << workload << std::endl;
    
    // 模拟计算工作
    volatile long sum = 0;
    for (int i = 0; i < workload * 100000; ++i) {
        sum += i;
    }
    
    std::cout << "CPU任务 " << taskId << " 完成" << std::endl;
}

// 模拟IO密集型任务
void ioIntensiveTask(int taskId, int sleepMs) {
    std::cout << "IO任务 " << taskId << " 开始执行，睡眠时间: " << sleepMs << "ms" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    
    std::cout << "IO任务 " << taskId << " 完成" << std::endl;
}

// 演示固定线程池
void demonstrateFixedThreadPool() {
    std::cout << "\n=== 固定线程池演示 ===" << std::endl;
    
    auto pool = createFixedThreadPool(4);
    pool->start();
    
    std::cout << "线程池类型: " << pool->getTypeName() << std::endl;
    std::cout << "提交10个CPU密集型任务..." << std::endl;
    
    // 提交CPU密集型任务
    for (int i = 0; i < 10; ++i) {
        pool->submit([i]() {
            cpuIntensiveTask(i, 50 + i * 10);
        });
    }
    
    // 等待任务完成
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // 显示统计信息
    auto stats = pool->getStats();
    std::cout << stats.toString() << std::endl;
    
    pool->shutdown();
    std::cout << "固定线程池演示完成\n" << std::endl;
}

// 演示缓存线程池
void demonstrateCachedThreadPool() {
    std::cout << "\n=== 缓存线程池演示 ===" << std::endl;
    
    auto pool = createCachedThreadPool();
    pool->start();
    
    std::cout << "线程池类型: " << pool->getTypeName() << std::endl;
    std::cout << "提交20个IO密集型任务..." << std::endl;
    
    // 提交IO密集型任务
    for (int i = 0; i < 20; ++i) {
        pool->submit([i]() {
            ioIntensiveTask(i, 100 + i * 50);
        });
    }
    
    // 等待任务完成
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 显示统计信息
    auto stats = pool->getStats();
    std::cout << stats.toString() << std::endl;
    
    pool->shutdown();
    std::cout << "缓存线程池演示完成\n" << std::endl;
}

// 演示优先级线程池
void demonstratePriorityThreadPool() {
    std::cout << "\n=== 优先级线程池演示 ===" << std::endl;
    
    auto pool = createPriorityThreadPool(3);
    pool->start();
    
    std::cout << "线程池类型: " << pool->getTypeName() << std::endl;
    
    // 类型转换以访问优先级功能
    auto priorityPool = dynamic_cast<PriorityThreadPool*>(pool.get());
    if (priorityPool) {
        std::cout << "提交不同优先级的任务..." << std::endl;
        
        // 提交低优先级任务
        for (int i = 0; i < 5; ++i) {
            priorityPool->submitWithPriority([i]() {
                std::cout << "低优先级任务 " << i << " 执行" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }, 1);
        }
        
        // 稍后提交高优先级任务
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        for (int i = 0; i < 3; ++i) {
            priorityPool->submitWithPriority([i]() {
                std::cout << "高优先级任务 " << i << " 执行" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }, 10);
        }
        
        // 等待任务完成
        std::this_thread::sleep_for(std::chrono::seconds(4));
        
        // 显示统计信息
        auto stats = pool->getStats();
        std::cout << stats.toString() << std::endl;
    }
    
    pool->shutdown();
    std::cout << "优先级线程池演示完成\n" << std::endl;
}

// 演示有返回值的任务
void demonstrateTaskWithResult() {
    std::cout << "\n=== 有返回值任务演示 ===" << std::endl;
    
    auto pool = createFixedThreadPool(2);
    pool->start();
    
    std::vector<std::future<int>> futures;
    
    // 提交计算任务
    for (int i = 1; i <= 5; ++i) {
        auto future = pool->submitWithResult([i]() -> int {
            std::cout << "计算任务 " << i << " 开始" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            int result = i * i;
            std::cout << "计算任务 " << i << " 完成，结果: " << result << std::endl;
            return result;
        });
        futures.push_back(std::move(future));
    }
    
    // 收集结果
    std::cout << "收集计算结果:" << std::endl;
    for (size_t i = 0; i < futures.size(); ++i) {
        try {
            int result = futures[i].get();
            std::cout << "任务 " << (i + 1) << " 结果: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "任务 " << (i + 1) << " 异常: " << e.what() << std::endl;
        }
    }
    
    pool->shutdown();
    std::cout << "有返回值任务演示完成\n" << std::endl;
}

// 演示拒绝策略
void demonstrateRejectionPolicy() {
    std::cout << "\n=== 拒绝策略演示 ===" << std::endl;
    
    // 创建一个小容量的线程池
    ThreadPoolConfig config(2, 5); // 2个线程，队列容量5
    auto pool = ThreadPoolFactory::create(ThreadPoolType::Fixed, config);
    
    auto fixedPool = dynamic_cast<FixedThreadPool*>(pool.get());
    if (fixedPool) {
        fixedPool->setRejectionPolicy(RejectionPolicy::Discard);
        pool->start();
        
        std::cout << "提交10个任务到容量为5的队列..." << std::endl;
        
        std::atomic<int> submittedCount{0};
        for (int i = 0; i < 10; ++i) {
            bool submitted = pool->submit([i]() {
                std::cout << "任务 " << i << " 执行中..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                std::cout << "任务 " << i << " 完成" << std::endl;
            });
            
            if (submitted) {
                ++submittedCount;
                std::cout << "任务 " << i << " 提交成功" << std::endl;
            } else {
                std::cout << "任务 " << i << " 被拒绝" << std::endl;
            }
        }
        
        std::cout << "成功提交 " << submittedCount.load() << " 个任务" << std::endl;
        
        // 等待任务完成
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        auto stats = pool->getStats();
        std::cout << stats.toString() << std::endl;
    }
    
    pool->shutdown();
    std::cout << "拒绝策略演示完成\n" << std::endl;
}

// 性能基准测试
void performanceBenchmark() {
    std::cout << "\n=== 性能基准测试 ===" << std::endl;
    
    std::vector<ThreadPoolType> types = {
        ThreadPoolType::Fixed,
        ThreadPoolType::Priority
    };
    
    auto results = ThreadPoolFactory::benchmark(types, 1000, std::chrono::milliseconds(3000));
    
    std::cout << "\n基准测试结果:" << std::endl;
    for (const auto& result : results) {
        std::cout << result.toString() << "\n" << std::endl;
    }
}

int main() {
    std::cout << "线程池框架演示程序" << std::endl;
    std::cout << "===================" << std::endl;
    
    try {
        // 显示系统信息
        std::cout << "系统硬件并发数: " << std::thread::hardware_concurrency() << std::endl;
        
        // 显示支持的线程池类型
        auto supportedTypes = ThreadPoolFactory::getSupportedTypes();
        std::cout << "支持的线程池类型:" << std::endl;
        for (auto type : supportedTypes) {
            std::cout << "  - " << ThreadPoolFactory::getTypeName(type) 
                      << ": " << ThreadPoolFactory::getTypeDescription(type) << std::endl;
        }
        
        // 各种演示
        demonstrateFixedThreadPool();
        // demonstrateCachedThreadPool();  // 需要实现CachedThreadPool
        demonstratePriorityThreadPool();
        demonstrateTaskWithResult();
        demonstrateRejectionPolicy();
        performanceBenchmark();
        
        std::cout << "\n所有演示完成！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 