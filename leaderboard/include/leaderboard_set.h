#ifndef LEADERBOARD_SET_H
#define LEADERBOARD_SET_H

#include "leaderboard.h"
#include <set>
#include <unordered_map>

namespace Leaderboard {

class LeaderboardSet : public ILeaderboard {
public:
    void addOrUpdate(const std::string& userId, int score) override;
    void remove(const std::string& userId) override;
    std::vector<Entry> topN(size_t n) const override;
    int getRank(const std::string& userId) const override;
    int getScore(const std::string& userId) const override;

private:
    // 使用 set 按分数降序排列，分数相同时按 userId 排序
    struct EntryComparator {
        bool operator()(const Entry& a, const Entry& b) const {
            if (a.score != b.score) {
                return a.score > b.score; // 分数高的在前
            }
            return a.userId < b.userId; // 分数相同时按 userId 字典序
        }
    };
    
    std::set<Entry, EntryComparator> entries_;
    std::unordered_map<std::string, int> userScores_; // 快速查找用户分数
};

} // namespace Leaderboard

#endif // LEADERBOARD_SET_H 