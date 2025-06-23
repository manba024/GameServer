#include "../include/leaderboard_persistent.h"
#include "../include/storage_file.h"
#include "../include/storage_sqlite.h"

namespace Leaderboard {

std::unique_ptr<IPersistentStorage> createStorage(StorageType type, const std::string& config) {
    switch (type) {
        case StorageType::File:
            return std::unique_ptr<IPersistentStorage>(new FileStorage(config));
        case StorageType::SQLite:
            return std::unique_ptr<IPersistentStorage>(new SQLiteStorage(config));
        case StorageType::Redis:
            // TODO: 实现Redis存储
            return nullptr;
        case StorageType::MySQL:
            // TODO: 实现MySQL存储
            return nullptr;
        default:
            return nullptr;
    }
}

std::unique_ptr<ILeaderboard> createPersistentLeaderboard(LeaderboardType lbType, 
                                                         StorageType storageType, 
                                                         const std::string& config) {
    // 创建基础排行榜实现
    auto baseLeaderboard = createLeaderboard(lbType);
    if (!baseLeaderboard) {
        return nullptr;
    }
    
    // 创建存储实现
    auto storage = createStorage(storageType, config);
    if (!storage) {
        return nullptr;
    }
    
    // 创建持久化排行榜
    return std::unique_ptr<ILeaderboard>(
        new PersistentLeaderboard(std::move(baseLeaderboard), std::move(storage))
    );
}

} // namespace Leaderboard 