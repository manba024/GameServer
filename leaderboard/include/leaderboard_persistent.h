#ifndef LEADERBOARD_PERSISTENT_H
#define LEADERBOARD_PERSISTENT_H

#include "leaderboard.h"
#include <string>
#include <memory>
#include <mutex>

namespace Leaderboard {

// 持久化存储接口
class IPersistentStorage {
public:
    virtual ~IPersistentStorage() = default;
    
    // 保存所有排行榜数据
    virtual bool saveAll(const std::vector<Entry>& entries) = 0;
    
    // 加载所有排行榜数据
    virtual std::vector<Entry> loadAll() = 0;
    
    // 增量保存单个条目
    virtual bool saveEntry(const Entry& entry) = 0;
    
    // 删除单个条目
    virtual bool removeEntry(const std::string& userId) = 0;
    
    // 批量保存多个条目
    virtual bool saveBatch(const std::vector<Entry>& entries) = 0;
    
    // 检查存储是否可用
    virtual bool isAvailable() const = 0;
    
    // 获取存储信息
    virtual std::string getStorageInfo() const = 0;
};

// 持久化排行榜基类
class PersistentLeaderboard : public ILeaderboard {
public:
    explicit PersistentLeaderboard(std::unique_ptr<ILeaderboard> impl, 
                                  std::unique_ptr<IPersistentStorage> storage);
    
    // 重写ILeaderboard接口，添加持久化功能
    void addOrUpdate(const std::string& userId, int score) override;
    void remove(const std::string& userId) override;
    std::vector<Entry> topN(size_t n) const override;
    int getRank(const std::string& userId) const override;
    int getScore(const std::string& userId) const override;
    
    // 持久化相关方法
    bool loadFromStorage();
    bool saveToStorage();
    bool enableAutoSave(bool enable);
    void setAutoSaveInterval(uint32_t intervalMs);
    std::string getStorageStatus() const;

private:
    std::unique_ptr<ILeaderboard> impl_;
    std::unique_ptr<IPersistentStorage> storage_;
    bool autoSave_;
    uint32_t autoSaveInterval_;
    mutable std::mutex mutex_;
    
    void autoSaveIfEnabled();
};

// 存储类型枚举
enum class StorageType {
    File,       // 文件存储
    SQLite,     // SQLite数据库
    Redis,      // Redis存储
    MySQL       // MySQL数据库
};

// 工厂方法
std::unique_ptr<IPersistentStorage> createStorage(StorageType type, const std::string& config);
std::unique_ptr<ILeaderboard> createPersistentLeaderboard(LeaderboardType lbType, 
                                                         StorageType storageType, 
                                                         const std::string& config);

} // namespace Leaderboard

#endif // LEADERBOARD_PERSISTENT_H 