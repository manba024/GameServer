#ifndef LEADERBOARD_HEAP_H
#define LEADERBOARD_HEAP_H

#include "leaderboard.h"
#include <vector>
#include <unordered_map>
#include <queue>

namespace Leaderboard {

class LeaderboardHeap : public ILeaderboard {
public:
    void addOrUpdate(const std::string& userId, int score) override;
    void remove(const std::string& userId) override;
    std::vector<Entry> topN(size_t n) const override;
    int getRank(const std::string& userId) const override;
    int getScore(const std::string& userId) const override;

private:
    struct EntryComparator {
        bool operator()(const Entry& a, const Entry& b) const {
            if (a.score != b.score) {
                return a.score < b.score; // 最大堆，分数高的在顶部
            }
            return a.userId > b.userId; // 分数相同时按 userId 字典序
        }
    };
    
    std::priority_queue<Entry, std::vector<Entry>, EntryComparator> heap_;
    std::unordered_map<std::string, int> userScores_;
    std::unordered_map<std::string, bool> removed_; // 标记已删除的用户
    
    void cleanupHeap(); // 清理已删除的条目
};

} // namespace Leaderboard

#endif // LEADERBOARD_HEAP_H 