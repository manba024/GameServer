#include "leaderboard_vector.h"

namespace Leaderboard {

// 添加或更新用户分数 更新后重新排序
void LeaderboardVector::addOrUpdate(const std::string& userId, int score) {
    for (auto& entry : entries_) {
        if (entry.userId == userId) {
            entry.score = score;
            std::sort(entries_.begin(), entries_.end(), [](const Entry& a, const Entry& b) {
                return a.score > b.score;
            });
            return;
        }
    }
    entries_.push_back({userId, score});
    std::sort(entries_.begin(), entries_.end(), [](const Entry& a, const Entry& b) {
        return a.score > b.score;
    });
}

void LeaderboardVector::remove(const std::string& userId) {
    entries_.erase(std::remove_if(entries_.begin(), entries_.end(), [&](const Entry& e){ return e.userId == userId; }), entries_.end());
}

std::vector<Entry> LeaderboardVector::topN(size_t n) const {
    if (n > entries_.size()) n = entries_.size();
    return std::vector<Entry>(entries_.begin(), entries_.begin() + n);
}

int LeaderboardVector::getRank(const std::string& userId) const {
    for (size_t i = 0; i < entries_.size(); ++i) {
        if (entries_[i].userId == userId) return i + 1;
    }
    return -1;
}

int LeaderboardVector::getScore(const std::string& userId) const {
    for (const auto& entry : entries_) {
        if (entry.userId == userId) return entry.score;
    }
    return -1;
}

} // namespace Leaderboard 