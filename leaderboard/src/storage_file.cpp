#include "../include/storage_file.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace Leaderboard {

FileStorage::FileStorage(const std::string& filename) : filename_(filename) {
}

bool FileStorage::saveAll(const std::vector<Entry>& entries) {
    std::lock_guard<std::mutex> lock(fileMutex_);
    
    // 创建备份
    if (fileExists()) {
        createBackup();
    }
    
    return writeToFile(entries);
}

std::vector<Entry> FileStorage::loadAll() {
    std::lock_guard<std::mutex> lock(fileMutex_);
    
    auto entries = readFromFile();
    if (entries.empty() && fileExists()) {
        // 尝试从备份恢复
        if (restoreFromBackup()) {
            entries = readFromFile();
        }
    }
    
    return entries;
}

bool FileStorage::saveEntry(const Entry& entry) {
    std::lock_guard<std::mutex> lock(fileMutex_);
    
    // 读取现有数据
    auto entries = readFromFile();
    
    // 更新或添加条目
    bool found = false;
    for (auto& e : entries) {
        if (e.userId == entry.userId) {
            e.score = entry.score;
            found = true;
            break;
        }
    }
    
    if (!found) {
        entries.push_back(entry);
    }
    
    return writeToFile(entries);
}

bool FileStorage::removeEntry(const std::string& userId) {
    std::lock_guard<std::mutex> lock(fileMutex_);
    
    // 读取现有数据
    auto entries = readFromFile();
    
    // 删除条目
    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
                      [&userId](const Entry& e) { return e.userId == userId; }),
        entries.end()
    );
    
    return writeToFile(entries);
}

bool FileStorage::saveBatch(const std::vector<Entry>& entries) {
    return saveAll(entries);
}

bool FileStorage::isAvailable() const {
    // 检查文件是否可写
    std::lock_guard<std::mutex> lock(fileMutex_);
    
    std::ofstream testFile(filename_, std::ios::app);
    bool available = testFile.good();
    testFile.close();
    
    return available;
}

std::string FileStorage::getStorageInfo() const {
    std::ostringstream oss;
    oss << "文件存储: " << filename_;
    
    if (fileExists()) {
        std::ifstream file(filename_);
        if (file.is_open()) {
            file.seekg(0, std::ios::end);
            auto size = file.tellg();
            oss << ", 大小: " << size << " 字节";
            file.close();
        }
    } else {
        oss << " (文件不存在)";
    }
    
    return oss.str();
}

bool FileStorage::writeToFile(const std::vector<Entry>& entries) {
    std::ofstream file(filename_, std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    
    // 写入格式：userId,score
    for (const auto& entry : entries) {
        file << entry.userId << "," << entry.score << "\n";
    }
    
    file.close();
    return file.good();
}

std::vector<Entry> FileStorage::readFromFile() {
    std::vector<Entry> entries;
    std::ifstream file(filename_);
    
    if (!file.is_open()) {
        return entries;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        // 解析格式：userId,score
        size_t commaPos = line.find(',');
        if (commaPos != std::string::npos) {
            std::string userId = line.substr(0, commaPos);
            std::string scoreStr = line.substr(commaPos + 1);
            
            try {
                int score = std::stoi(scoreStr);
                entries.push_back({userId, score});
            } catch (const std::exception& e) {
                // 跳过无效行
                std::cerr << "解析排行榜数据失败: " << line << std::endl;
            }
        }
    }
    
    file.close();
    return entries;
}

bool FileStorage::fileExists() const {
    std::ifstream file(filename_);
    return file.good();
}

bool FileStorage::createBackup() {
    std::string backupName = filename_ + ".backup";
    
    std::ifstream src(filename_, std::ios::binary);
    std::ofstream dst(backupName, std::ios::binary);
    
    if (!src.is_open() || !dst.is_open()) {
        return false;
    }
    
    dst << src.rdbuf();
    
    src.close();
    dst.close();
    
    return dst.good();
}

bool FileStorage::restoreFromBackup() {
    std::string backupName = filename_ + ".backup";
    
    std::ifstream src(backupName, std::ios::binary);
    std::ofstream dst(filename_, std::ios::binary);
    
    if (!src.is_open() || !dst.is_open()) {
        return false;
    }
    
    dst << src.rdbuf();
    
    src.close();
    dst.close();
    
    return dst.good();
}

} // namespace Leaderboard 