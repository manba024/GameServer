#ifndef STORAGE_SQLITE_H
#define STORAGE_SQLITE_H

#include "leaderboard_persistent.h"
#include <sqlite3.h>
#include <mutex>

namespace Leaderboard {

class SQLiteStorage : public IPersistentStorage {
public:
    explicit SQLiteStorage(const std::string& dbPath);
    ~SQLiteStorage();
    
    bool saveAll(const std::vector<Entry>& entries) override;
    std::vector<Entry> loadAll() override;
    bool saveEntry(const Entry& entry) override;
    bool removeEntry(const std::string& userId) override;
    bool saveBatch(const std::vector<Entry>& entries) override;
    bool isAvailable() const override;
    std::string getStorageInfo() const override;

private:
    sqlite3* db_;
    std::string dbPath_;
    mutable std::mutex dbMutex_;
    
    // 辅助方法
    bool initDatabase();
    bool createTable();
    bool executeSQL(const std::string& sql);
    std::string escapeString(const std::string& str);
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
};

} // namespace Leaderboard

#endif // STORAGE_SQLITE_H 