#include "../include/leaderboard_persistent.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace Leaderboard;

void demonstrateBasicPersistence() {
    std::cout << "=== 基础持久化功能演示 ===\n";
    
    // 创建持久化排行榜：Set + 文件存储
    auto leaderboard = createPersistentLeaderboard(
        LeaderboardType::Set,
        StorageType::File,
        "demo_leaderboard.txt"
    );
    
    if (!leaderboard) {
        std::cout << "创建排行榜失败！\n";
        return;
    }
    
    std::cout << "1. 添加测试数据...\n";
    leaderboard->addOrUpdate("张三", 1000);
    leaderboard->addOrUpdate("李四", 950);
    leaderboard->addOrUpdate("王五", 1100);
    leaderboard->addOrUpdate("赵六", 800);
    leaderboard->addOrUpdate("孙七", 1200);
    
    std::cout << "当前Top3排行榜：\n";
    auto top3 = leaderboard->topN(3);
    for (size_t i = 0; i < top3.size(); i++) {
        std::cout << (i+1) << ". " << top3[i].userId << ": " << top3[i].score << "分\n";
    }
    
    std::cout << "\n2. 手动保存数据到文件...\n";
    auto* persistent = dynamic_cast<PersistentLeaderboard*>(leaderboard.get());
    if (persistent) {
        bool saved = persistent->saveToStorage();
        std::cout << "保存结果: " << (saved ? "成功" : "失败") << "\n";
        std::cout << "存储状态: " << persistent->getStorageStatus() << "\n";
    }
    
    std::cout << "\n3. 重新创建排行榜（模拟程序重启）...\n";
    auto newLeaderboard = createPersistentLeaderboard(
        LeaderboardType::Set,
        StorageType::File,
        "demo_leaderboard.txt"
    );
    
    std::cout << "重新加载后的Top3排行榜：\n";
    auto loadedTop3 = newLeaderboard->topN(3);
    for (size_t i = 0; i < loadedTop3.size(); i++) {
        std::cout << (i+1) << ". " << loadedTop3[i].userId << ": " << loadedTop3[i].score << "分\n";
    }
    
    // 验证数据一致性
    bool consistent = (top3.size() == loadedTop3.size());
    if (consistent) {
        for (size_t i = 0; i < top3.size(); i++) {
            if (top3[i].userId != loadedTop3[i].userId || 
                top3[i].score != loadedTop3[i].score) {
                consistent = false;
                break;
            }
        }
    }
    
    std::cout << "\n数据一致性检查: " << (consistent ? "✓ 通过" : "✗ 失败") << "\n";
}

void demonstrateAutoSave() {
    std::cout << "\n=== 自动保存功能演示 ===\n";
    
    auto leaderboard = createPersistentLeaderboard(
        LeaderboardType::Set,
        StorageType::File,
        "demo_autosave.txt"
    );
    
    auto* persistent = dynamic_cast<PersistentLeaderboard*>(leaderboard.get());
    if (!persistent) {
        std::cout << "转换为持久化排行榜失败！\n";
        return;
    }
    
    // 启用自动保存，间隔2秒
    persistent->enableAutoSave(true);
    persistent->setAutoSaveInterval(2000);
    
    std::cout << "启用自动保存功能（间隔2秒）\n";
    std::cout << "开始逐步添加数据...\n";
    
    // 逐步添加数据，观察自动保存
    std::string players[] = {"小明", "小红", "小刚", "小丽", "小华"};
    int scores[] = {100, 200, 150, 300, 250};
    
    for (int i = 0; i < 5; i++) {
        leaderboard->addOrUpdate(players[i], scores[i]);
        std::cout << "添加 " << players[i] << " (" << scores[i] << "分)\n";
        
        // 等待1秒，让自动保存有机会触发
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "\n等待最后一次自动保存...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    std::cout << "最终排行榜：\n";
    auto finalTop = leaderboard->topN(5);
    for (size_t i = 0; i < finalTop.size(); i++) {
        std::cout << (i+1) << ". " << finalTop[i].userId << ": " << finalTop[i].score << "分\n";
    }
    
    std::cout << "存储状态: " << persistent->getStorageStatus() << "\n";
}

void demonstrateSQLiteStorage() {
    std::cout << "\n=== SQLite存储演示 ===\n";
    
    auto leaderboard = createPersistentLeaderboard(
        LeaderboardType::SkipList,
        StorageType::SQLite,
        "demo_leaderboard.db"
    );
    
    if (!leaderboard) {
        std::cout << "创建SQLite排行榜失败！可能需要安装SQLite3库\n";
        return;
    }
    
    std::cout << "使用SkipList + SQLite存储\n";
    
    // 添加一些测试数据
    std::cout << "添加测试数据...\n";
    leaderboard->addOrUpdate("数据库用户1", 1500);
    leaderboard->addOrUpdate("数据库用户2", 1200);
    leaderboard->addOrUpdate("数据库用户3", 1800);
    leaderboard->addOrUpdate("数据库用户4", 1100);
    
    auto* persistent = dynamic_cast<PersistentLeaderboard*>(leaderboard.get());
    if (persistent) {
        std::cout << "存储状态: " << persistent->getStorageStatus() << "\n";
    }
    
    std::cout << "SQLite存储的Top3：\n";
    auto top3 = leaderboard->topN(3);
    for (size_t i = 0; i < top3.size(); i++) {
        std::cout << (i+1) << ". " << top3[i].userId << ": " << top3[i].score << "分\n";
    }
    
    std::cout << "数据库用户2的排名: " << leaderboard->getRank("数据库用户2") << "\n";
    std::cout << "数据库用户3的分数: " << leaderboard->getScore("数据库用户3") << "分\n";
}

void demonstrateGameScenario() {
    std::cout << "\n=== 游戏场景演示 ===\n";
    
    // 模拟游戏中的多个排行榜
    auto playerRanking = createPersistentLeaderboard(
        LeaderboardType::SkipList,
        StorageType::File,
        "game_players.txt"
    );
    
    auto guildRanking = createPersistentLeaderboard(
        LeaderboardType::Set,
        StorageType::File,
        "game_guilds.txt"
    );
    
    std::cout << "创建了玩家排行榜和公会排行榜\n";
    
    // 配置自动保存
    auto* playerPersistent = dynamic_cast<PersistentLeaderboard*>(playerRanking.get());
    auto* guildPersistent = dynamic_cast<PersistentLeaderboard*>(guildRanking.get());
    
    if (playerPersistent) {
        playerPersistent->enableAutoSave(true);
        playerPersistent->setAutoSaveInterval(5000); // 5秒
    }
    
    if (guildPersistent) {
        guildPersistent->enableAutoSave(true);
        guildPersistent->setAutoSaveInterval(10000); // 10秒
    }
    
    // 模拟游戏数据
    std::cout << "\n模拟游戏数据更新...\n";
    
    // 玩家数据
    playerRanking->addOrUpdate("战士小明", 15000);
    playerRanking->addOrUpdate("法师小红", 18000);
    playerRanking->addOrUpdate("弓手小刚", 12000);
    playerRanking->addOrUpdate("牧师小丽", 16000);
    
    // 公会数据
    guildRanking->addOrUpdate("龙腾公会", 500000);
    guildRanking->addOrUpdate("凤舞公会", 450000);
    guildRanking->addOrUpdate("虎啸公会", 480000);
    
    std::cout << "玩家排行榜 Top3：\n";
    auto playerTop = playerRanking->topN(3);
    for (size_t i = 0; i < playerTop.size(); i++) {
        std::cout << (i+1) << ". " << playerTop[i].userId << ": " << playerTop[i].score << "战力\n";
    }
    
    std::cout << "\n公会排行榜 Top3：\n";
    auto guildTop = guildRanking->topN(3);
    for (size_t i = 0; i < guildTop.size(); i++) {
        std::cout << (i+1) << ". " << guildTop[i].userId << ": " << guildTop[i].score << "积分\n";
    }
    
    // 模拟分数更新
    std::cout << "\n模拟战斗后分数更新...\n";
    playerRanking->addOrUpdate("战士小明", 16000); // 小明战力提升
    playerRanking->addOrUpdate("新手小张", 8000);  // 新玩家加入
    
    std::cout << "更新后的玩家排行榜 Top4：\n";
    auto updatedTop = playerRanking->topN(4);
    for (size_t i = 0; i < updatedTop.size(); i++) {
        std::cout << (i+1) << ". " << updatedTop[i].userId << ": " << updatedTop[i].score << "战力\n";
    }
    
    std::cout << "\n战士小明的最新排名: " << playerRanking->getRank("战士小明") << "\n";
}

int main() {
    std::cout << "排行榜持久化功能演示程序\n";
    std::cout << "========================\n";
    
    try {
        // 基础持久化演示
        demonstrateBasicPersistence();
        
        // 自动保存演示
        demonstrateAutoSave();
        
        // SQLite存储演示
        demonstrateSQLiteStorage();
        
        // 游戏场景演示
        demonstrateGameScenario();
        
        std::cout << "\n=== 演示完成 ===\n";
        std::cout << "生成的文件：\n";
        std::cout << "- demo_leaderboard.txt (基础演示)\n";
        std::cout << "- demo_autosave.txt (自动保存演示)\n";
        std::cout << "- demo_leaderboard.db (SQLite演示)\n";
        std::cout << "- game_players.txt (游戏玩家排行榜)\n";
        std::cout << "- game_guilds.txt (游戏公会排行榜)\n";
        
        std::cout << "\n你可以查看这些文件来了解数据的持久化格式。\n";
        std::cout << "重新运行程序可以看到数据被正确加载。\n";
        
    } catch (const std::exception& e) {
        std::cerr << "演示过程中发生错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 