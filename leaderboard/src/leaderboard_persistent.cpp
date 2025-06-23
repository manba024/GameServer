#include "../include/leaderboard_persistent.h"
#include <thread>
#include <chrono>
#include <climits>

namespace Leaderboard {

PersistentLeaderboard::PersistentLeaderboard(std::unique_ptr<ILeaderboard> impl, 
                                           std::unique_ptr<IPersistentStorage> storage)
    : impl_(std::move(impl)), storage_(std::move(storage)), 
      autoSave_(false), autoSaveInterval_(5000) {
    // 启动时自动加载数据
    loadFromStorage();
}

void PersistentLeaderboard::addOrUpdate(const std::string& userId, int score) {
    std::lock_guard<std::mutex> lock(mutex_);
    impl_->addOrUpdate(userId, score);
    
    // 增量保存
    Entry entry = {userId, score};
    storage_->saveEntry(entry);
    
    autoSaveIfEnabled();
}

void PersistentLeaderboard::remove(const std::string& userId) {
    std::lock_guard<std::mutex> lock(mutex_);
    impl_->remove(userId);
    
    // 从存储中删除
    storage_->removeEntry(userId);
    
    autoSaveIfEnabled();
}

std::vector<Entry> PersistentLeaderboard::topN(size_t n) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->topN(n);
}

int PersistentLeaderboard::getRank(const std::string& userId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->getRank(userId);
}

int PersistentLeaderboard::getScore(const std::string& userId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->getScore(userId);
}

bool PersistentLeaderboard::loadFromStorage() {
    if (!storage_ || !storage_->isAvailable()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto entries = storage_->loadAll();
    for (const auto& entry : entries) {
        impl_->addOrUpdate(entry.userId, entry.score);
    }
    
    return true;
}

bool PersistentLeaderboard::saveToStorage() {
    if (!storage_ || !storage_->isAvailable()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 获取所有数据并保存 - 使用一个很大的数字来获取所有条目
    auto allEntries = impl_->topN(1000000); // 假设不会超过100万用户
    return storage_->saveAll(allEntries);
}

bool PersistentLeaderboard::enableAutoSave(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    autoSave_ = enable;
    return true;
}

void PersistentLeaderboard::setAutoSaveInterval(uint32_t intervalMs) {
    std::lock_guard<std::mutex> lock(mutex_);
    autoSaveInterval_ = intervalMs;
}

std::string PersistentLeaderboard::getStorageStatus() const {
    if (!storage_) {
        return "存储未初始化";
    }
    
    std::string status = storage_->getStorageInfo();
    status += ", 自动保存: " + std::string(autoSave_ ? "开启" : "关闭");
    if (autoSave_) {
        status += ", 间隔: " + std::to_string(autoSaveInterval_) + "ms";
    }
    
    return status;
}

void PersistentLeaderboard::autoSaveIfEnabled() {
    if (!autoSave_) {
        return;
    }
    
    // 启动异步保存线程
    static auto lastSave = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSave);
    
    if (elapsed.count() >= autoSaveInterval_) {
        std::thread([this]() {
            this->saveToStorage();
        }).detach();
        lastSave = now;
    }
}

} // namespace Leaderboard 