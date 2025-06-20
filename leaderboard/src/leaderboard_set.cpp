#include "leaderboard_set.h"

namespace Leaderboard {

void LeaderboardSet::addOrUpdate(const std::string& userId, int score) {
    // 如果用户已存在，先删除旧记录
    auto it = userScores_.find(userId);
    if (it != userScores_.end()) {
        Entry oldEntry = {userId, it->second};
        entries_.erase(oldEntry);
    }
    
    // 添加新记录
    Entry newEntry = {userId, score};
    entries_.insert(newEntry);
    userScores_[userId] = score;
}

void LeaderboardSet::remove(const std::string& userId) {
    auto it = userScores_.find(userId);
    if (it != userScores_.end()) {
        Entry entry = {userId, it->second};
        entries_.erase(entry);
        userScores_.erase(it);
    }
}

std::vector<Entry> LeaderboardSet::topN(size_t n) const {
    std::vector<Entry> result;
    result.reserve(std::min(n, entries_.size()));
    
    auto it = entries_.begin();
    for (size_t i = 0; i < n && it != entries_.end(); ++i, ++it) {
        result.push_back(*it);
    }
    
    return result;
}

int LeaderboardSet::getRank(const std::string& userId) const {
    auto scoreIt = userScores_.find(userId);
    if (scoreIt == userScores_.end()) {
        return -1; // 用户不存在
    }
    
    Entry userEntry = {userId, scoreIt->second};
    int rank = 1;
    
    for (const auto& entry : entries_) {
        if (entry.userId == userId && entry.score == scoreIt->second) {
            return rank;
        }
        rank++;
    }
    
    return -1; // 不应该到达这里
}

int LeaderboardSet::getScore(const std::string& userId) const {
    auto it = userScores_.find(userId);
    if (it != userScores_.end()) {
        return it->second;
    }
    return -1; // 用户不存在
}

} // namespace Leaderboard 