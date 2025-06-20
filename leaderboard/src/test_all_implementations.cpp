#include "leaderboard.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

using namespace Leaderboard;

void testLeaderboard(std::unique_ptr<ILeaderboard> lb, const std::string& typeName) {
    std::cout << "\n=== 测试 " << typeName << " 实现 ===\n";
    
    // 测试添加用户
    lb->addOrUpdate("Alice", 100);
    lb->addOrUpdate("Bob", 85);
    lb->addOrUpdate("Charlie", 120);
    lb->addOrUpdate("David", 95);
    lb->addOrUpdate("Eve", 110);
    
    std::cout << "添加5个用户后:\n";
    
    // 测试 topN
    auto top3 = lb->topN(3);
    std::cout << "Top 3: ";
    for (const auto& entry : top3) {
        std::cout << entry.userId << "(" << entry.score << ") ";
    }
    std::cout << "\n";
    
    // 测试排名查询
    std::cout << "Alice 排名: " << lb->getRank("Alice") << "\n";
    std::cout << "Bob 排名: " << lb->getRank("Bob") << "\n";
    std::cout << "Charlie 排名: " << lb->getRank("Charlie") << "\n";
    
    // 测试分数查询
    std::cout << "Alice 分数: " << lb->getScore("Alice") << "\n";
    std::cout << "不存在用户分数: " << lb->getScore("NonExistent") << "\n";
    
    // 测试更新分数
    lb->addOrUpdate("Alice", 130);
    std::cout << "Alice 更新分数到130后排名: " << lb->getRank("Alice") << "\n";
    
    // 测试删除用户
    lb->remove("Bob");
    std::cout << "删除Bob后，Bob排名: " << lb->getRank("Bob") << "\n";
    std::cout << "删除Bob后Top 5: ";
    auto top5 = lb->topN(5);
    for (const auto& entry : top5) {
        std::cout << entry.userId << "(" << entry.score << ") ";
    }
    std::cout << "\n";
}

void performanceTest(LeaderboardType type, const std::string& typeName, int userCount) {
    std::cout << "\n=== " << typeName << " 性能测试 (用户数: " << userCount << ") ===\n";
    
    auto lb = createLeaderboard(type);
    auto start = std::chrono::high_resolution_clock::now();
    
    // 添加用户
    for (int i = 0; i < userCount; i++) {
        lb->addOrUpdate("user" + std::to_string(i), rand() % 10000);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "添加 " << userCount << " 个用户耗时: " << duration.count() << "ms\n";
    
    // 查询性能测试
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        lb->getRank("user" + std::to_string(i % userCount));
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "1000次排名查询耗时: " << duration.count() << "ms\n";
    
    // TopN 查询性能
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++) {
        lb->topN(10);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "100次Top10查询耗时: " << duration.count() << "ms\n";
}

int main() {
    std::cout << "排行榜多实现测试程序\n";
    std::cout << "====================\n";
    
    // 功能测试
    testLeaderboard(createLeaderboard(LeaderboardType::Vector), "Vector");
    testLeaderboard(createLeaderboard(LeaderboardType::Set), "Set");
    testLeaderboard(createLeaderboard(LeaderboardType::Heap), "Heap");
    testLeaderboard(createLeaderboard(LeaderboardType::SkipList), "SkipList");
    
    // 性能测试
    std::cout << "\n\n性能对比测试\n";
    std::cout << "============\n";
    
    // 小规模测试 (1000用户)
    performanceTest(LeaderboardType::Vector, "Vector", 1000);
    performanceTest(LeaderboardType::Set, "Set", 1000);
    performanceTest(LeaderboardType::Heap, "Heap", 1000);
    performanceTest(LeaderboardType::SkipList, "SkipList", 1000);
    
    // 中等规模测试 (10000用户)
    performanceTest(LeaderboardType::Set, "Set", 10000);
    performanceTest(LeaderboardType::Heap, "Heap", 10000);
    performanceTest(LeaderboardType::SkipList, "SkipList", 10000);
    
    std::cout << "\n测试完成！\n";
    return 0;
} 