#include "../include/leaderboard_persistent.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

using namespace Leaderboard;

void testPersistentLeaderboard(LeaderboardType lbType, StorageType storageType, 
                              const std::string& config, const std::string& testName) {
    std::cout << "\n=== 测试 " << testName << " ===\n";
    
    // 创建持久化排行榜
    auto lb = createPersistentLeaderboard(lbType, storageType, config);
    if (!lb) {
        std::cout << "创建排行榜失败!\n";
        return;
    }
    
    // 转换为持久化排行榜以访问额外功能
    PersistentLeaderboard* persistentLB = dynamic_cast<PersistentLeaderboard*>(lb.get());
    if (persistentLB) {
        std::cout << "存储状态: " << persistentLB->getStorageStatus() << "\n";
        
        // 启用自动保存
        persistentLB->enableAutoSave(true);
        persistentLB->setAutoSaveInterval(2000); // 2秒间隔
    }
    
    std::cout << "\n--- 第一阶段：添加数据 ---\n";
    
    // 添加测试数据
    lb->addOrUpdate("Alice", 100);
    lb->addOrUpdate("Bob", 85);
    lb->addOrUpdate("Charlie", 120);
    lb->addOrUpdate("David", 95);
    lb->addOrUpdate("Eve", 110);
    
    std::cout << "添加5个用户后的Top3:\n";
    auto top3 = lb->topN(3);
    for (size_t i = 0; i < top3.size(); i++) {
        std::cout << (i+1) << ". " << top3[i].userId << ": " << top3[i].score << "\n";
    }
    
    // 手动保存
    if (persistentLB) {
        std::cout << "\n手动保存到存储...\n";
        bool saved = persistentLB->saveToStorage();
        std::cout << "保存结果: " << (saved ? "成功" : "失败") << "\n";
    }
    
    std::cout << "\n--- 第二阶段：重新创建排行榜（测试加载） ---\n";
    
    // 重新创建排行榜，测试数据加载
    auto lb2 = createPersistentLeaderboard(lbType, storageType, config);
    if (!lb2) {
        std::cout << "重新创建排行榜失败!\n";
        return;
    }
    
    std::cout << "重新加载后的Top3:\n";
    auto top3_loaded = lb2->topN(3);
    for (size_t i = 0; i < top3_loaded.size(); i++) {
        std::cout << (i+1) << ". " << top3_loaded[i].userId << ": " << top3_loaded[i].score << "\n";
    }
    
    // 验证数据一致性
    bool dataConsistent = (top3.size() == top3_loaded.size());
    if (dataConsistent) {
        for (size_t i = 0; i < top3.size(); i++) {
            if (top3[i].userId != top3_loaded[i].userId || 
                top3[i].score != top3_loaded[i].score) {
                dataConsistent = false;
                break;
            }
        }
    }
    
    std::cout << "数据一致性检查: " << (dataConsistent ? "通过" : "失败") << "\n";
    
    std::cout << "\n--- 第三阶段：更新和删除操作 ---\n";
    
    // 更新分数
    lb2->addOrUpdate("Alice", 130);
    std::cout << "更新Alice分数到130后排名: " << lb2->getRank("Alice") << "\n";
    
    // 删除用户
    lb2->remove("Bob");
    std::cout << "删除Bob后，Bob排名: " << lb2->getRank("Bob") << "\n";
    
    std::cout << "最终Top5:\n";
    auto top5 = lb2->topN(5);
    for (size_t i = 0; i < top5.size(); i++) {
        std::cout << (i+1) << ". " << top5[i].userId << ": " << top5[i].score << "\n";
    }
    
    // 最终保存
    PersistentLeaderboard* persistentLB2 = dynamic_cast<PersistentLeaderboard*>(lb2.get());
    if (persistentLB2) {
        persistentLB2->saveToStorage();
        std::cout << "\n最终存储状态: " << persistentLB2->getStorageStatus() << "\n";
    }
}

void testFileStorage() {
    std::cout << "\n======= 文件存储测试 =======\n";
    
    // 测试不同排行榜实现 + 文件存储
    testPersistentLeaderboard(LeaderboardType::Vector, StorageType::File, 
                             "leaderboard_vector.txt", "Vector + 文件存储");
    
    testPersistentLeaderboard(LeaderboardType::Set, StorageType::File, 
                             "leaderboard_set.txt", "Set + 文件存储");
    
    testPersistentLeaderboard(LeaderboardType::SkipList, StorageType::File, 
                             "leaderboard_skiplist.txt", "SkipList + 文件存储");
}

void testSQLiteStorage() {
    std::cout << "\n======= SQLite存储测试 =======\n";
    
    // 测试不同排行榜实现 + SQLite存储
    testPersistentLeaderboard(LeaderboardType::Vector, StorageType::SQLite, 
                             "leaderboard_vector.db", "Vector + SQLite存储");
    
    testPersistentLeaderboard(LeaderboardType::Set, StorageType::SQLite, 
                             "leaderboard_set.db", "Set + SQLite存储");
    
    testPersistentLeaderboard(LeaderboardType::SkipList, StorageType::SQLite, 
                             "leaderboard_skiplist.db", "SkipList + SQLite存储");
}

void testPerformance() {
    std::cout << "\n======= 持久化性能测试 =======\n";
    
    const int userCount = 1000;
    
    // 文件存储性能测试
    {
        std::cout << "\n--- 文件存储性能 ---\n";
        auto lb = createPersistentLeaderboard(LeaderboardType::Set, StorageType::File, 
                                             "performance_test.txt");
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < userCount; i++) {
            lb->addOrUpdate("user" + std::to_string(i), rand() % 10000);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "插入 " << userCount << " 个用户耗时: " << duration.count() << "ms\n";
        
        // 测试查询性能
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 100; i++) {
            lb->topN(10);
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "100次Top10查询耗时: " << duration.count() << "ms\n";
    }
    
    // SQLite存储性能测试
    {
        std::cout << "\n--- SQLite存储性能 ---\n";
        auto lb = createPersistentLeaderboard(LeaderboardType::Set, StorageType::SQLite, 
                                             "performance_test.db");
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < userCount; i++) {
            lb->addOrUpdate("user" + std::to_string(i), rand() % 10000);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "插入 " << userCount << " 个用户耗时: " << duration.count() << "ms\n";
        
        // 测试查询性能
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 100; i++) {
            lb->topN(10);
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "100次Top10查询耗时: " << duration.count() << "ms\n";
    }
}

void testAutoSave() {
    std::cout << "\n======= 自动保存测试 =======\n";
    
    auto lb = createPersistentLeaderboard(LeaderboardType::Set, StorageType::File, 
                                         "autosave_test.txt");
    
    PersistentLeaderboard* persistentLB = dynamic_cast<PersistentLeaderboard*>(lb.get());
    if (!persistentLB) {
        std::cout << "转换为持久化排行榜失败!\n";
        return;
    }
    
    // 启用自动保存，间隔1秒
    persistentLB->enableAutoSave(true);
    persistentLB->setAutoSaveInterval(1000);
    
    std::cout << "启用自动保存，间隔1秒\n";
    std::cout << "添加数据中...\n";
    
    // 逐步添加数据，观察自动保存
    for (int i = 0; i < 10; i++) {
        lb->addOrUpdate("user" + std::to_string(i), rand() % 1000);
        std::cout << "添加用户 user" << i << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::cout << "等待最后一次自动保存...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "自动保存测试完成\n";
    std::cout << "存储状态: " << persistentLB->getStorageStatus() << "\n";
}

int main() {
    std::cout << "排行榜持久化测试程序\n";
    std::cout << "====================\n";
    
    // 文件存储测试
    testFileStorage();
    
    // SQLite存储测试（需要安装SQLite3）
    testSQLiteStorage();
    
    // 性能测试
    testPerformance();
    
    // 自动保存测试
    testAutoSave();
    
    std::cout << "\n所有测试完成！\n";
    std::cout << "\n生成的测试文件:\n";
    std::cout << "- leaderboard_*.txt (文件存储测试)\n";
    std::cout << "- leaderboard_*.db (SQLite存储测试)\n";
    std::cout << "- performance_test.* (性能测试)\n";
    std::cout << "- autosave_test.txt (自动保存测试)\n";
    
    return 0;
} 