#include "../include/id_generator.h"
#include <iostream>
#include <vector>
#include <set>
#include <chrono>
#include <thread>

using namespace DistributedId;

void testGenerator(std::unique_ptr<IIdGenerator> generator, const std::string& name, int count = 10) {
    std::cout << "\n=== 测试 " << name << " ===\n";
    std::cout << "生成器信息: " << generator->getGeneratorInfo() << "\n";
    std::cout << "支持数字ID: " << (generator->supportsNumericId() ? "是" : "否") << "\n\n";
    
    // 生成示例ID
    std::cout << "生成 " << count << " 个ID示例:\n";
    std::set<std::string> stringIds;
    std::set<uint64_t> numericIds;
    
    for (int i = 0; i < count; i++) {
        std::string strId = generator->generateStringId();
        std::cout << "字符串ID: " << strId << "\n";
        stringIds.insert(strId);
        
        if (generator->supportsNumericId()) {
            uint64_t numId = generator->generateNumericId();
            std::cout << "数字ID:   " << numId << "\n";
            numericIds.insert(numId);
        }
        std::cout << "---\n";
        
        // 短暂延迟，避免时间戳相同
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // 检查唯一性
    std::cout << "唯一性检查:\n";
    std::cout << "字符串ID唯一数量: " << stringIds.size() << "/" << count << "\n";
    if (generator->supportsNumericId()) {
        std::cout << "数字ID唯一数量: " << numericIds.size() << "/" << count << "\n";
    }
}

void performanceTest(IdGeneratorType type, const std::string& name, int count = 100000) {
    std::cout << "\n=== " << name << " 性能测试 ===\n";
    std::cout << "生成 " << count << " 个ID...\n";
    
    auto generator = createIdGenerator(type);
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < count; i++) {
        generator->generateStringId();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "耗时: " << duration.count() << "ms\n";
    std::cout << "平均每秒生成: " << (count * 1000) / (duration.count() + 1) << " 个ID\n";
}

void testSnowflakeWithDifferentConfigs() {
    std::cout << "\n=== 测试不同配置的雪花算法 ===\n";
    
    // 配置1: 工作节点1，数据中心1
    SnowflakeConfig config1(1, 1);
    auto generator1 = createSnowflakeGenerator(config1);
    std::cout << "配置1 - " << generator1->getGeneratorInfo() << "\n";
    for (int i = 0; i < 3; i++) {
        std::cout << "ID: " << generator1->generateNumericId() << "\n";
    }
    
    // 配置2: 工作节点2，数据中心1  
    SnowflakeConfig config2(2, 1);
    auto generator2 = createSnowflakeGenerator(config2);
    std::cout << "\n配置2 - " << generator2->getGeneratorInfo() << "\n";
    for (int i = 0; i < 3; i++) {
        std::cout << "ID: " << generator2->generateNumericId() << "\n";
    }
}

void testConcurrency() {
    std::cout << "\n=== 并发测试 ===\n";
    
    auto generator = createIdGenerator(IdGeneratorType::Snowflake);
    const int threadCount = 4;
    const int idsPerThread = 1000;
    std::vector<std::thread> threads;
    std::vector<std::vector<uint64_t>> results(threadCount);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 启动多个线程并发生成ID
    for (int t = 0; t < threadCount; t++) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < idsPerThread; i++) {
                results[t].push_back(generator->generateNumericId());
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 检查唯一性
    std::set<uint64_t> allIds;
    for (const auto& threadResults : results) {
        for (uint64_t id : threadResults) {
            allIds.insert(id);
        }
    }
    
    int totalIds = threadCount * idsPerThread;
    std::cout << "并发生成 " << totalIds << " 个ID\n";
    std::cout << "唯一ID数量: " << allIds.size() << "\n";
    std::cout << "唯一性: " << ((int)allIds.size() == totalIds ? "通过" : "失败") << "\n";
    std::cout << "耗时: " << duration.count() << "ms\n";
}

int main() {
    std::cout << "分布式全局唯一ID生成器测试程序\n";
    std::cout << "================================\n";
    
    // 功能测试
    testGenerator(createIdGenerator(IdGeneratorType::UUID), "UUID生成器");
    testGenerator(createIdGenerator(IdGeneratorType::Snowflake), "雪花算法生成器");
    testGenerator(createIdGenerator(IdGeneratorType::Timestamp), "时间戳生成器");
    
    // 雪花算法不同配置测试
    testSnowflakeWithDifferentConfigs();
    
    // 性能测试
    std::cout << "\n\n性能对比测试\n";
    std::cout << "============\n";
    performanceTest(IdGeneratorType::UUID, "UUID", 50000);
    performanceTest(IdGeneratorType::Snowflake, "雪花算法", 100000);
    performanceTest(IdGeneratorType::Timestamp, "时间戳", 100000);
    
    // 并发测试
    testConcurrency();
    
    std::cout << "\n测试完成！\n";
    return 0;
} 