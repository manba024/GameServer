#include "../include/storage_sqlite.h"
#include <iostream>
#include <sstream>

namespace Leaderboard {

SQLiteStorage::SQLiteStorage(const std::string& dbPath) : db_(nullptr), dbPath_(dbPath) {
    initDatabase();
}

SQLiteStorage::~SQLiteStorage() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool SQLiteStorage::saveAll(const std::vector<Entry>& entries) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    
    if (!db_ || !beginTransaction()) {
        return false;
    }
    
    // 清空表
    if (!executeSQL("DELETE FROM leaderboard")) {
        rollbackTransaction();
        return false;
    }
    
    // 批量插入
    const char* sql = "INSERT INTO leaderboard (user_id, score) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        rollbackTransaction();
        return false;
    }
    
    bool success = true;
    for (const auto& entry : entries) {
        sqlite3_bind_text(stmt, 1, entry.userId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, entry.score);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            success = false;
            break;
        }
        
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    
    if (success) {
        commitTransaction();
    } else {
        rollbackTransaction();
    }
    
    return success;
}

std::vector<Entry> SQLiteStorage::loadAll() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Entry> entries;
    
    if (!db_) {
        return entries;
    }
    
    const char* sql = "SELECT user_id, score FROM leaderboard ORDER BY score DESC";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return entries;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* userId = (const char*)sqlite3_column_text(stmt, 0);
        int score = sqlite3_column_int(stmt, 1);
        
        if (userId) {
            entries.push_back({std::string(userId), score});
        }
    }
    
    sqlite3_finalize(stmt);
    return entries;
}

bool SQLiteStorage::saveEntry(const Entry& entry) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    
    if (!db_) {
        return false;
    }
    
    // 使用 INSERT OR REPLACE 语句
    const char* sql = "INSERT OR REPLACE INTO leaderboard (user_id, score) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, entry.userId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, entry.score);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

bool SQLiteStorage::removeEntry(const std::string& userId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    
    if (!db_) {
        return false;
    }
    
    const char* sql = "DELETE FROM leaderboard WHERE user_id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

bool SQLiteStorage::saveBatch(const std::vector<Entry>& entries) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    
    if (!db_ || !beginTransaction()) {
        return false;
    }
    
    const char* sql = "INSERT OR REPLACE INTO leaderboard (user_id, score) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        rollbackTransaction();
        return false;
    }
    
    bool success = true;
    for (const auto& entry : entries) {
        sqlite3_bind_text(stmt, 1, entry.userId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, entry.score);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            success = false;
            break;
        }
        
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    
    if (success) {
        commitTransaction();
    } else {
        rollbackTransaction();
    }
    
    return success;
}

bool SQLiteStorage::isAvailable() const {
    std::lock_guard<std::mutex> lock(dbMutex_);
    return db_ != nullptr;
}

std::string SQLiteStorage::getStorageInfo() const {
    std::ostringstream oss;
    oss << "SQLite存储: " << dbPath_;
    
    if (db_) {
        // 获取记录数量
        const char* sql = "SELECT COUNT(*) FROM leaderboard";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int count = sqlite3_column_int(stmt, 0);
                oss << ", 记录数: " << count;
            }
            sqlite3_finalize(stmt);
        }
    } else {
        oss << " (数据库未连接)";
    }
    
    return oss.str();
}

bool SQLiteStorage::initDatabase() {
    if (sqlite3_open(dbPath_.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "无法打开SQLite数据库: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    return createTable();
}

bool SQLiteStorage::createTable() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS leaderboard (
            user_id TEXT PRIMARY KEY,
            score INTEGER NOT NULL,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return executeSQL(sql);
}

bool SQLiteStorage::executeSQL(const std::string& sql) {
    char* errorMsg = nullptr;
    int result = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMsg);
    
    if (result != SQLITE_OK) {
        std::cerr << "SQL执行失败: " << errorMsg << std::endl;
        sqlite3_free(errorMsg);
        return false;
    }
    
    return true;
}

std::string SQLiteStorage::escapeString(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.length() * 2);
    
    for (char c : str) {
        if (c == '\'') {
            escaped += "''";
        } else {
            escaped += c;
        }
    }
    
    return escaped;
}

bool SQLiteStorage::beginTransaction() {
    return executeSQL("BEGIN TRANSACTION");
}

bool SQLiteStorage::commitTransaction() {
    return executeSQL("COMMIT");
}

bool SQLiteStorage::rollbackTransaction() {
    return executeSQL("ROLLBACK");
}

} // namespace Leaderboard 