#ifndef LEADERBOARD_SKIPLIST_H
#define LEADERBOARD_SKIPLIST_H

#include "leaderboard.h"
#include <vector>
#include <unordered_map>
#include <random>

namespace Leaderboard {

class LeaderboardSkipList : public ILeaderboard {
public:
    LeaderboardSkipList();
    ~LeaderboardSkipList();
    
    void addOrUpdate(const std::string& userId, int score) override;
    void remove(const std::string& userId) override;
    std::vector<Entry> topN(size_t n) const override;
    int getRank(const std::string& userId) const override;
    int getScore(const std::string& userId) const override;

private:
    static const int MAX_LEVEL = 16; // 最大层数
    
    struct SkipListNode {
        Entry entry;
        std::vector<SkipListNode*> forward;
        
        SkipListNode(const Entry& e, int level) : entry(e), forward(level + 1, nullptr) {}
    };
    
    SkipListNode* header_;
    int level_; // 当前最高层数
    std::unordered_map<std::string, SkipListNode*> userNodes_; // 快速查找用户节点
    std::mt19937 rng_;
    
    int randomLevel();
    bool shouldGoHigher(const Entry& a, const Entry& b) const; // 比较函数
    SkipListNode* find(const std::string& userId) const;
    void deleteNode(SkipListNode* node);
};

} // namespace Leaderboard

#endif // LEADERBOARD_SKIPLIST_H 