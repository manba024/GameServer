#include "leaderboard_heap.h"

namespace Leaderboard {

void LeaderboardHeap::addOrUpdate(const std::string& userId, int score) {
    // 如果用户已存在，标记旧记录为已删除
    if (userScores_.find(userId) != userScores_.end()) {
        removed_[userId] = true;
    }
    
    // 添加新记录
    Entry newEntry = {userId, score};
    heap_.push(newEntry);
    userScores_[userId] = score;
    removed_[userId] = false;
    
    // 定期清理堆
    if (removed_.size() > heap_.size() / 2) {
        cleanupHeap();
    }
}

void LeaderboardHeap::remove(const std::string& userId) {
    auto it = userScores_.find(userId);
    if (it != userScores_.end()) {
        removed_[userId] = true;
        userScores_.erase(it);
    }
}

std::vector<Entry> LeaderboardHeap::topN(size_t n) const {
    std::vector<Entry> result;
    result.reserve(n);
    
    // 创建堆的副本进行操作
    auto tempHeap = heap_;
    
    while (result.size() < n && !tempHeap.empty()) {
        Entry top = tempHeap.top();
        tempHeap.pop();
        
        // 检查是否是有效记录（未被删除且分数匹配）
        auto scoreIt = userScores_.find(top.userId);
        auto removedIt = removed_.find(top.userId);
        
        if (scoreIt != userScores_.end() && 
            scoreIt->second == top.score &&
            (removedIt == removed_.end() || !removedIt->second)) {
            result.push_back(top);
        }
    }
    
    return result;
}

int LeaderboardHeap::getRank(const std::string& userId) const {
    auto scoreIt = userScores_.find(userId);
    if (scoreIt == userScores_.end()) {
        return -1; // 用户不存在
    }
    
    int userScore = scoreIt->second;
    int rank = 1;
    
    // 创建堆的副本进行遍历
    auto tempHeap = heap_;
    
    while (!tempHeap.empty()) {
        Entry top = tempHeap.top();
        tempHeap.pop();
        
        // 检查是否是有效记录
        auto currentScoreIt = userScores_.find(top.userId);
        auto removedIt = removed_.find(top.userId);
        
        if (currentScoreIt != userScores_.end() && 
            currentScoreIt->second == top.score &&
            (removedIt == removed_.end() || !removedIt->second)) {
            
            if (top.userId == userId) {
                return rank;
            }
            
            if (top.score > userScore || 
                (top.score == userScore && top.userId < userId)) {
                rank++;
            }
        }
    }
    
    return rank;
}

int LeaderboardHeap::getScore(const std::string& userId) const {
    auto it = userScores_.find(userId);
    if (it != userScores_.end()) {
        return it->second;
    }
    return -1; // 用户不存在
}

void LeaderboardHeap::cleanupHeap() {
    std::vector<Entry> validEntries;
    
    // 提取所有有效条目
    auto tempHeap = heap_;
    while (!tempHeap.empty()) {
        Entry entry = tempHeap.top();
        tempHeap.pop();
        
        auto scoreIt = userScores_.find(entry.userId);
        auto removedIt = removed_.find(entry.userId);
        
        if (scoreIt != userScores_.end() && 
            scoreIt->second == entry.score &&
            (removedIt == removed_.end() || !removedIt->second)) {
            validEntries.push_back(entry);
        }
    }
    
    // 重建堆
    heap_ = std::priority_queue<Entry, std::vector<Entry>, EntryComparator>();
    for (const auto& entry : validEntries) {
        heap_.push(entry);
    }
    
    // 清理已删除标记
    for (auto it = removed_.begin(); it != removed_.end();) {
        if (it->second) { // 如果标记为已删除
            it = removed_.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace Leaderboard 