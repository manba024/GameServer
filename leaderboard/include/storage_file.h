#ifndef STORAGE_FILE_H
#define STORAGE_FILE_H

#include "leaderboard_persistent.h"
#include <fstream>
#include <mutex>

namespace Leaderboard {

class FileStorage : public IPersistentStorage {
public:
    explicit FileStorage(const std::string& filename);
    ~FileStorage() = default;
    
    bool saveAll(const std::vector<Entry>& entries) override;
    std::vector<Entry> loadAll() override;
    bool saveEntry(const Entry& entry) override;
    bool removeEntry(const std::string& userId) override;
    bool saveBatch(const std::vector<Entry>& entries) override;
    bool isAvailable() const override;
    std::string getStorageInfo() const override;

private:
    std::string filename_;
    mutable std::mutex fileMutex_;
    
    // 辅助方法
    bool writeToFile(const std::vector<Entry>& entries);
    std::vector<Entry> readFromFile();
    bool fileExists() const;
    bool createBackup();
    bool restoreFromBackup();
};

} // namespace Leaderboard

#endif // STORAGE_FILE_H 